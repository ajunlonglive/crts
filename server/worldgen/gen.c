#include <math.h>

#include <stdlib.h>
#include <string.h>

#include "server/sim/terrain.h"
#include "server/worldgen/gen.h"
#include "shared/math/delaunay.h"
#include "shared/math/geom.h"
#include "shared/math/kernel_filter.h"
#include "shared/math/linalg.h"
#include "shared/math/perlin.h"
#include "shared/math/rand.h"
#include "shared/math/triangle.h"
#include "shared/types/darr.h"
#include "shared/util/log.h"

struct terrain_vertex {
	const struct pointf *p;
	const struct tg_edge *faultedge;
	float elev;
	uint8_t fault;
	uint32_t filled;
	vec4 norm;
};

/* put all floats at the top */
static const uint32_t terrain_pixel_depth = 3;
struct terrain_pixel {
	float elev,
	      watershed,
	      waterstop;

	float x, y;

	enum tile t;
	uint32_t overdraw;
	bool stream;
	vec4 norm;
};

struct terrain {
	struct worldgen_opts opts;
	float height, width;
	float max_watershed, max_waterstop;
	uint8_t faults;
	struct hdarr *tdat;
	struct darr *fault_points;
	struct pointf mid;
	float radius;
	struct terrain_pixel *heightmap;
};

static struct terrain_pixel *
get_terrain_pix(struct terrain *terra, uint32_t x, uint32_t y)
{
	uint32_t index = (y * terra->width) + x;
	assert(index < terra->height * terra->width);
	return &terra->heightmap[index];
}

static void
seed_terrain_data(struct trigraph *tg, struct terrain *terra)
{
	uint32_t i;

	perlin_noise_shuf();

	for (i = 0; i < darr_len(tg->points); ++i) {
		struct pointf *p = darr_get(tg->points, i);

		float distmod = ((1.0 - (fsqdist(&terra->mid, p) / terra->radius)) - 0.5);

		float elev = distmod - 2;
		elev = -5;

		struct terrain_vertex td = { .p = p, .elev = elev };
		hdarr_set(terra->tdat, p, &td);
		hdarr_get(terra->tdat, p);
	}
}

static void
gen_fault(struct trigraph *tg, struct terrain *terra)
{
	bool first_pass = true;
	const uint8_t fault_id = ++terra->faults;
	const struct tg_edge *e = darr_get(hdarr_darr(tg->edges),
		rand_uniform(hdarr_len(tg->edges))), *oe = e;
	struct tg_tri *t;
	const struct pointf *p;

	/* float boost = rand_uniform(40) - 20; */
	float boost, oboost = rand_chance(terra->opts.fault_valley_chance)
		? (terra->opts.fault_valley_max - rand_uniform(terra->opts.fault_valley_mod))
		: (rand_uniform(terra->opts.fault_mtn_mod) + terra->opts.fault_valley_min);

gen_fault_pass:
	boost = oboost;
	p = first_pass ? oe->a : oe->b;

	uint32_t i = 0;
	while (++i < terra->opts.fault_max_len
	       && fsqdist(p, &terra->mid) < terra->radius * terra->opts.fault_radius_pct_extent) {
		float angle = 0.0, nextang;

		struct terrain_vertex *tdat[] = {
			hdarr_get(terra->tdat, e->a),
			hdarr_get(terra->tdat, e->b)
		};

		if (p == e->a) {
			if (tdat[1]->fault) {
				break;
			}
			p = e->b;
		} else {
			if (tdat[0]->fault) {
				break;
			}
			p = e->a;
		}

		darr_push(terra->fault_points, p);

		if (!tdat[0]->fault) {
			tdat[0]->fault = fault_id;
			tdat[0]->filled = fault_id;
			tdat[0]->elev += boost;
			tdat[0]->faultedge = e;
		}

		if (!tdat[1]->fault) {
			tdat[1]->fault = fault_id;
			tdat[1]->filled = fault_id;
			tdat[1]->elev += boost;
			tdat[1]->faultedge = e;
		}

		t = hdarr_get(tg->tris, e->adja);
		while (angle < PI) {
			nextang = tg_point_angle(t, p);

			if (angle + nextang > terra->opts.fault_max_ang && angle > 0.0) {
				break;
			}

			angle += nextang;

			const struct tg_edge *n = next_edge(tg, t, e, p);

			e = n;
			if (tg_tris_eql(t, e->adja) && e->adjb[0]) {
				t = hdarr_get(tg->tris, e->adjb);
			} else {
				t = hdarr_get(tg->tris, e->adja);
			}
		}

		boost -= boost > 0 ? 1.0 : -1.0;
	}

	if (first_pass) {
		first_pass = false;
		goto gen_fault_pass;
	}
}

struct fill_plates_recursor_ctx {
	struct trigraph *tg;
	struct terrain *terra;
	const struct pointf *start;
	float closest;
	float boost;
	uint32_t id;
	float boost_decay;
};

static void
fill_plates_recursor(const struct pointf *p, const struct tg_edge *e, void *_ctx)
{
	const struct fill_plates_recursor_ctx *ctx = _ctx;
	struct terrain_vertex *td;
	float dist;

	if ((dist = fsqdist(p, ctx->start)) < ctx->closest) {
		return;
	} else if ((td = hdarr_get(ctx->terra->tdat, p))->filled & ctx->id || td->fault) {
		return;
	}

	/* td->elev += ctx->boost * (10 / sqrt(dist)); */
	td->elev += ctx->boost;
	td->filled |= ctx->id;

	if (fabs(ctx->boost) < 1) {
		return;
	}

	struct fill_plates_recursor_ctx new_ctx = *ctx;
	new_ctx.closest = dist;
	new_ctx.boost = ctx->boost * ctx->boost_decay;

	tg_for_each_adjacent_point(ctx->tg, p, e, &new_ctx, fill_plates_recursor);
}

static void
fill_plates(struct trigraph *tg, struct terrain *terra)
{
	uint32_t i;
	for (i = 0; i < darr_len(terra->fault_points); ++i) {
		struct terrain_vertex *td = hdarr_get(terra->tdat, darr_get(terra->fault_points, i));
		assert(td->fault);

		struct fill_plates_recursor_ctx ctx = {
			.tg = tg,
			.terra = terra,
			.start = td->p,
			.closest = 0,
			.boost = td->elev,
			.id = td->fault,
			.boost_decay = terra->opts.fault_boost_decay,
		};

		tg_for_each_adjacent_point(tg, td->p, td->faultedge, &ctx, fill_plates_recursor);
	}
}

static void
rasterize_tri_cb(void *_terra, float *vd_interp, size_t vd_len, int32_t x, int32_t y)
{
	struct terrain *terra = _terra;

	if (x < 0 || x >= terra->width || y < 0 || y >= terra->width) {
		return;
	}

	struct terrain_pixel *tp = get_terrain_pix(terra, x, y);

	tp->elev = vd_interp[0];

	tp->norm[0] = vd_interp[1];
	tp->norm[1] = vd_interp[2];
	tp->norm[2] = vd_interp[3];

	tp->x = x, tp->y = y;

	++tp->overdraw;

}

static void
rasterize_terrain(struct trigraph *tg, struct terrain *terra)
{
	uint32_t i;
	for (i = 0; i < hdarr_len(tg->tris); ++i) {
		struct tg_tri *t = darr_get(hdarr_darr(tg->tris), i);

		struct terrain_vertex *tdat[] = {
			hdarr_get(terra->tdat, t->a),
			hdarr_get(terra->tdat, t->b),
			hdarr_get(terra->tdat, t->c)
		};

		assert(tdat[0] && tdat[1] && tdat[2]);

		vec4 points3d[3] = {
			{ t->a->x, tdat[0]->elev, t->a->y, },
			{ t->c->x, tdat[2]->elev, t->c->y, },
			{ t->b->x, tdat[1]->elev, t->b->y, },
		};

		vec4 norm;
		calc_normal(points3d[0], points3d[1], points3d[2], norm);

		vec4_add(tdat[0]->norm, norm);
		vec4_add(tdat[1]->norm, norm);
		vec4_add(tdat[2]->norm, norm);
	}

	for (i = 0; i < hdarr_len(terra->tdat); ++i) {
		struct terrain_vertex *tdat = darr_get(hdarr_darr(terra->tdat), i);

		vec4_normalize(tdat->norm);
	}

	for (i = 0; i < hdarr_len(tg->tris); ++i) {
		struct tg_tri *t = darr_get(hdarr_darr(tg->tris), i);

		struct terrain_vertex *tdat[] = {
			hdarr_get(terra->tdat, t->a),
			hdarr_get(terra->tdat, t->b),
			hdarr_get(terra->tdat, t->c)
		};

		assert(tdat[0] && tdat[1] && tdat[2]);

		float vertex_data[][3] = {
			{ tdat[0]->elev, tdat[1]->elev, tdat[2]->elev, },
			{ tdat[0]->norm[0], tdat[1]->norm[0], tdat[2]->norm[0], },
			{ tdat[0]->norm[1], tdat[1]->norm[1], tdat[2]->norm[1], },
			{ tdat[0]->norm[2], tdat[1]->norm[2], tdat[2]->norm[2], },
		};

		rasterize_tri(t, terra, vertex_data, 4, rasterize_tri_cb);
	}
}


static void
trace_raindrop(struct terrain *terra, float x, float y)
{
	float sediment = 0;
	float vx = 0;
	float vy = 0;

	struct terrain_pixel *tp = NULL, *ptp = NULL;

	uint32_t i;
	for (i = 0; i < terra->opts.raindrop_max_iterations; ++i) {
		int32_t xi = roundf(x), yi = roundf(y);

		if (xi < 1 || xi >= terra->width - 1 || yi < 1 || yi >= terra->height - 1) {
			break;
		}

		tp = get_terrain_pix(terra, xi, yi);

		assert(tp);

		if (ptp == NULL) {
			ptp = tp;
		}

		if (tp->norm[1] == 1 || tp->elev < -2) {
			++tp->waterstop;
			break;
		}

		float deposit = sediment * terra->opts.deposition_rate * tp->norm[1];
		float erosion = terra->opts.erosion_rate * (1 - tp->norm[1]);
		float height_diff = deposit - erosion;

		/* L("norm: %f, %f, %f", tp->norm[0], tp->norm[1], tp->norm[2]); */
		/* L("pos: (%f, %f, %f), sed: %f, deposit: %f, erosion: %f, hdiff: %f, v: %f, %f", */
		/* 	x, tp->elev, y, sediment, deposit, erosion, height_diff, vx, vy); */
		/* get_terrain_pix(terra, xi - 1, yi)->elev += height_diff; */
		/* get_terrain_pix(terra, xi + 1, yi)->elev += height_diff; */
		/* get_terrain_pix(terra, xi, yi)->elev += height_diff; */
		/* get_terrain_pix(terra, xi, yi - 1)->elev += height_diff; */
		/* get_terrain_pix(terra, xi, yi + 1)->elev += height_diff; */

		if (ptp->elev < tp->elev) {
			/* L("in the pit"); */
			++tp->waterstop;

			sediment -= tp->elev - ptp->elev;

			ptp->elev += deposit;
			vx = vy = 0;

			if (sediment < 0) {
				break;
			}
		} else {
			sediment += erosion - deposit;
			ptp->elev += height_diff;
		}

		++ptp->watershed;

		vx = terra->opts.raindrop_friction * vx + tp->norm[0] * terra->opts.raindrop_speed;
		vy = terra->opts.raindrop_friction * vy + tp->norm[2] * terra->opts.raindrop_speed;
		x += vx;
		y += vy;

		ptp = tp;
	}

	if (tp) {
		++tp->waterstop;

		if (tp->waterstop / terra->opts.raindrops < 0.0008
		    && tp->waterstop > terra->max_waterstop) {
			terra->max_waterstop = tp->waterstop;
		}

		if (tp->watershed / terra->opts.raindrops < 0.001
		    && tp->watershed > terra->max_watershed) {
			terra->max_watershed = tp->watershed;
		}
	}
}

static void
trace_river(struct terrain *terra, struct terrain_pixel *cur)
{
	/* float sediment = 0; */
	float vx = 0;
	float vy = 0;

	struct terrain_pixel *next = NULL;

	float x = cur->x, y = cur->y;

	uint32_t i;
	for (i = 0; i < terra->opts.raindrop_max_iterations; ++i) {
		int32_t xi = roundf(x), yi = roundf(y);

		if (xi < 1 || xi >= terra->width - 1 || yi < 1 || yi >= terra->height - 1) {
			break;
		}

		vx = terra->opts.raindrop_friction * vx + cur->norm[0] * terra->opts.raindrop_speed;
		vy = terra->opts.raindrop_friction * vy + cur->norm[2] * terra->opts.raindrop_speed;
		x += vx;
		y += vy;

		next = get_terrain_pix(terra, xi, yi);

		assert(next);

		if (next->elev < -4) {
			break;
		}

		/* float deposit = sediment * terra->opts.deposition_rate * tp->norm[1]; */
		/* float erosion = terra->opts.erosion_rate * (1 - tp->norm[1]); */
		/* float height_diff = deposit - erosion; */

		cur->stream = true;
		/* if (next->elev > cur->elev) { */
		/* 	/1* L("in the pit"); *1/ */
		/* 	next->elev = cur->elev; */
		/* 	/1* } else { *1/ */
		/* 	/1* 	sediment += erosion - deposit; *1/ */
		/* 	/1* 	ptp->elev += height_diff; *1/ */
		/* } */

		cur = next;

		/* ++ptp->watershed; */

		/* ptp = tp; */
	}
}

static void
simulate_erosion(struct terrain *terra)
{
	uint32_t i;
	for (i = 0; i < terra->opts.raindrops; ++i) {
		trace_raindrop(terra, rand_uniform(terra->width), rand_uniform(terra->height));
		float pct = (float)i / terra->opts.raindrops * 100.0;
		if (i % 100000 == 0) {
			L("%f%% done", pct);
		}
	}

	L("max watershed: %f, avg: %f, max waterstop: %f",
		terra->max_watershed, terra->max_watershed / terra->opts.raindrops, terra->max_waterstop);

	uint32_t rx, ry;
	for (ry = 0; ry < terra->height; ++ry) {
		for (rx = 0; rx < terra->width; ++rx) {
			struct terrain_pixel *tp = &terra->heightmap[ry * (uint32_t)terra->width + rx];

			if ((tp->watershed /= terra->max_watershed) > 1.0) {
				tp->watershed = 1.0;
			}

			if ((tp->waterstop /= terra->max_waterstop) > 1.0) {
				tp->waterstop = 1.0;
			}

			if (tp->waterstop >= 0.20) {
				trace_river(terra, tp);
			}
		}
	}
}

static void
add_noise(struct terrain *terra)
{
	uint32_t rx, ry;

	for (ry = 0; ry < terra->height; ++ry) {
		for (rx = 0; rx < terra->width; ++rx) {
			struct terrain_pixel *tp = &terra->heightmap[ry * (uint32_t)terra->width + rx];

			tp->elev += perlin_two(rx, ry, terra->opts.final_noise_amp,
				terra->opts.final_noise_octs,
				terra->opts.final_noise_freq,
				terra->opts.final_noise_lacu);
		}
	}
}

static void
gaussian_blur(struct terrain *terra)
{
	float *grid = calloc(terra->height * terra->width,
		sizeof(float) * terrain_pixel_depth),
	      kernel[7];
	uint32_t rx, ry;

	for (ry = 0; ry < terra->height; ++ry) {
		for (rx = 0; rx < terra->width; ++rx) {
			memcpy(&grid[(ry * terra->opts.width + rx) * terrain_pixel_depth],
				&terra->heightmap[ry * terra->opts.width + rx],
				sizeof(float) * terrain_pixel_depth);
		}
	}

	gen_gaussian_kernel(kernel, 1.0, 7);
	convolve_seperable_kernel(grid, terra->height, terra->width,
		terrain_pixel_depth, kernel, 7);

	for (ry = 0; ry < terra->height; ++ry) {
		for (rx = 0; rx < terra->width; ++rx) {
			memcpy(&terra->heightmap[ry * terra->opts.width + rx],
				&grid[(ry * terra->opts.width + rx) * terrain_pixel_depth],
				sizeof(float) * terrain_pixel_depth);
		}
	}

	free(grid);
}

static void
write_chunks(struct chunks *chunks, struct terrain *terra)
{
	struct point p = { 0, 0 }, q;

	assert((uint32_t)terra->width % CHUNK_SIZE == 0);
	assert((uint32_t)terra->height % CHUNK_SIZE == 0);

	uint32_t maxdraw = 0, rx, ry;
	for (p.x = 0; p.x < terra->width; p.x += CHUNK_SIZE) {
		for (p.y = 0; p.y < terra->height; p.y += CHUNK_SIZE) {
			struct chunk *ck = get_chunk(chunks, &p);

			for (rx = 0; rx < CHUNK_SIZE; ++rx) {
				for (ry = 0; ry < CHUNK_SIZE; ++ry) {
					q.x = p.x + rx;
					q.y = p.y + ry;

					struct terrain_pixel *tp =
						&terra->heightmap[(uint32_t)((q.y * terra->width) + q.x)];

					if (tp->stream) {
						ck->tiles[rx][ry] = tile_stream;
						goto set_elev;
					}

					if (tp->elev < -5) {
						ck->tiles[rx][ry] = tile_deep_water;
					} else if (tp->elev < 0) {
						ck->tiles[rx][ry] = tile_water;
					} else if (tp->elev < 3) {
						if (tp->watershed < 0.5) {
							ck->tiles[rx][ry] = tile_wetland;
						} else if (tp->watershed < 0.6) {
							ck->tiles[rx][ry] = tile_wetland_forest_old;
						} else {
							ck->tiles[rx][ry] = tile_wetland_forest;
						}
					} else if (tp->elev < 30) {
						if (tp->watershed < 0.3) {
							ck->tiles[rx][ry] = tile_plain;
						} else if (tp->watershed < 0.4) {
							ck->tiles[rx][ry] = tile_forest_old;
						} else {
							ck->tiles[rx][ry] = tile_forest;
						}
					} else if (tp->elev < 40) {
						ck->tiles[rx][ry] = tile_mountain;
					} else {
						ck->tiles[rx][ry] = tile_peak;
					}

set_elev:

					ck->heights[rx][ry] = tp->elev;

					if (tp->overdraw > maxdraw) {
						maxdraw = tp->overdraw;
					}
				}
			}
		}
	}

	/* L("max overdraw was %d", maxdraw); */
}

void
gen_terrain(struct chunks *chunks, struct worldgen_opts *opts)
{
	struct trigraph tg = { 0 };
	struct pointf corner = { 0.0, 0.0 };
	struct terrain terra = {
		.opts = *opts,
		.height = opts->height,
		.width = opts->width,
		.mid = { opts->width * 0.5, opts->height * 0.5 },
		.tdat = hdarr_init(2048, sizeof(struct pointf *),
			sizeof(struct terrain_vertex), NULL),
		.fault_points = darr_init(sizeof(struct pointf *)),
		.heightmap = calloc(opts->height * opts->width,
			sizeof(struct terrain_pixel)),
	};
	uint32_t i;

	rand_set_seed(opts->seed);

	terra.radius = fsqdist(&terra.mid, &corner) * terra.opts.radius;

	trigraph_init(&tg);
	L("scattering seed points");
	tg_scatter(&tg, terra.opts.width, terra.opts.height, terra.opts.points, true);

	L("finding delaunay triangulation");
	delaunay(&tg);

	L("seeding terrain");
	seed_terrain_data(&tg, &terra);

	L("generating fault lines");
	for (i = 0; i < terra.opts.faults; ++i) {
		gen_fault(&tg, &terra);
	}

	L("filling tectonic plates");
	fill_plates(&tg, &terra);

	L("rasterizing terrain heightmap");
	rasterize_terrain(&tg, &terra);

	L("simulating erosion");
	simulate_erosion(&terra);

	L("adding noise");
	add_noise(&terra);

	L("applying gaussian blur");
	gaussian_blur(&terra);

	L("writing chunks");
	write_chunks(chunks, &terra);

	free(terra.heightmap);
	hdarr_destroy(terra.tdat);
	darr_destroy(terra.fault_points);
	/* TODO: tgraph_destroy(&tg) */
}
