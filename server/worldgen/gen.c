#include <math.h>

#include <stdlib.h>
#include <string.h>

#include "server/sim/terrain.h"
#include "server/worldgen/gen.h"
#include "shared/math/delaunay.h"
#include "shared/math/geom.h"
#include "shared/math/linalg.h"
#include "shared/math/perlin.h"
#include "shared/math/rand.h"
#include "shared/types/darr.h"
#include "shared/util/log.h"

static void
cartesian_to_barycentric(const struct pointf *a, const struct pointf *b,
	const struct pointf *c,
	float x, float y, float *l1, float *l2, float *l3)
{
	assert(signed_area(a, b, c) > 0);

	float x1 = a->x, y1 = a->y,
	      x2 = b->x, y2 = b->y,
	      x3 = c->x, y3 = c->y,
	      det = (y2 - y3) * (x1 - x3) + (x3 - x2) * (y1 - y3);

	*l1 = ((y2 - y3) * (x - x3) + (x3 - x2) * (y - y3)) / det;
	*l2 = ((y3 - y1) * (x - x3) + (x1 - x3) * (y - y3)) / det;
	*l3 = 1.0f - (*l1 + *l2);
}

static void
draw_point(struct chunks *chunks, float h, enum tile tt, const struct pointf *cc)
{
	struct point p = { (int)roundf(cc->x), (int)roundf(cc->y) };
	struct point np = nearest_chunk(&p);
	struct chunk *ck = get_chunk(chunks, &np);
	struct point rp = point_sub(&p, &ck->pos);

	ck->heights[rp.x][rp.y] = h;

	if (tt >= 0 && tt < tile_count) {
		ck->tiles[rp.x][rp.y] = tt;
	}
}

void
draw_edge(struct chunks *chunks, enum tile t, float h, const struct tg_edge *e)
{
	const struct pointf *p = e->a, *q = e->b;
	struct pointf r = *p;

	float dx = q->x - p->x, dy = q->y - p->y,
	      l = fabs(dx) > fabs(dy) ? fabs(dx) : fabs(dy), i;

	dx /= l;
	dy /= l;

	for (i = 0; i < l; ++i) {
		draw_point(chunks, t, h, &r);
		r.x += dx;
		r.y += dy;
	}
}

struct terrain_data {
	const struct pointf *p;
	const struct tg_edge *faultedge;
	float elev;
	uint8_t fault;
	uint32_t filled;
	vec4 norm;
};

struct terrain_pixel {
	float elev;
	enum tile t;
	uint32_t overdraw;
	uint32_t watershed, waterstop;
	vec4 norm;
};

struct terrain {
	struct worldgen_opts opts;
	float height, width;
	uint8_t faults;
	struct hdarr *tdat;
	struct darr *fault_points;
	struct pointf mid;
	float radius;
	struct terrain_pixel *heightmap;
};

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

		struct terrain_data *tdat[] = {
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
	struct terrain_data *td;
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
		struct terrain_data *td = hdarr_get(terra->tdat, darr_get(terra->fault_points, i));
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


static struct terrain_pixel *
get_terrain_pix(struct terrain *terra, uint32_t x, uint32_t y)
{
	uint32_t index = (y * terra->width) + x;
	assert(index < terra->height * terra->width);
	return &terra->heightmap[index];
}

static void
trace_raindrop(struct terrain *terra, float x, float y)
{
	float sediment = 0; // The amount of carried sediment
	float vx = 0; // The horizontal velocity
	float vy = 0; // The vertical velocity

	struct terrain_pixel *tp, *ptp = NULL;
	/* L("simulating raindrop"); */

	uint32_t i;
	for (i = 0; i < terra->opts.raindrop_max_iterations; ++i) {
		int32_t xi = roundf(x), yi = roundf(y);

		if (xi < 1 || xi >= terra->width - 1 || yi < 1 || yi >= terra->height - 1) {
			/* L("running off the map"); */
			break;
		}

		tp = get_terrain_pix(terra, xi, yi);

		assert(tp);

		if (ptp == NULL) {
			ptp = tp;
		}

		if (tp->norm[1] == 1 || tp->elev < -2) {
			/* L("dying: %f, %f", tp->norm[1], tp->elev); */
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
		/* ptp->elev -= 0.001; */
		++ptp->watershed;

		vx = terra->opts.raindrop_friction * vx + tp->norm[0] * terra->opts.raindrop_speed;
		vy = terra->opts.raindrop_friction * vy + tp->norm[2] * terra->opts.raindrop_speed;
		x += vx;
		y += vy;

		ptp = tp;
	}
	/* L("iterations: %d", i); */
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

		struct terrain_data td = { .p = p, .elev = elev };
		hdarr_set(terra->tdat, p, &td);
		hdarr_get(terra->tdat, p);
	}
}

static void
sort_vertices(const struct pointf *a, const struct pointf *b,
	const struct pointf *c, const struct pointf **low,
	const struct pointf **mid, const struct pointf **hi)
{
	if (a->y < b->y) {
		*low = a;
		*mid = b;
	} else {
		*low = b;
		*mid = a;
	}

	if ((*mid)->y > c->y) {
		*hi = *mid;
		if ((*low)->y > c->y) {
			*mid = *low;
			*low = c;
		} else {
			*mid = c;
		}
	} else {
		*hi = c;
	}
}

struct tri_vertex_data {
	const struct pointf *points[3];
	struct terrain_data *tdat[3];
	vec4 norm;
};

static void
draw_line(struct terrain *terra, const struct tri_vertex_data *t,
	float x1, float y1, float x2, float y2)
{
	assert(y1 == y2);

	int32_t iy = roundf(y1), ix;
	if (iy < 0 || iy >= terra->height) {
		return;
	}

	if (x1 < x2) {
		float tmp = x2;
		x2 = x1;
		x1 = tmp;
	}

	x1 += 1;

	while (x1 >= x2) {
		ix = roundf(x1);

		if (ix < 0 || ix >= terra->width) {
			x1 -= 1;
			continue;
		}

		float t1 = 0, t2 = 0, t3 = 0;

		cartesian_to_barycentric(t->points[0], t->points[1], t->points[2],
			x1, y1, &t1, &t2, &t3);

		struct terrain_pixel *tp = get_terrain_pix(terra, ix, iy);

		tp->elev = t1 * t->tdat[0]->elev + t2 * t->tdat[1]->elev + t3 * t->tdat[2]->elev;

		tp->norm[0] = t1 * t->tdat[0]->norm[0] + t2 * t->tdat[1]->norm[0] + t3 * t->tdat[2]->norm[0];
		tp->norm[1] = t1 * t->tdat[0]->norm[1] + t2 * t->tdat[1]->norm[1] + t3 * t->tdat[2]->norm[1];
		tp->norm[2] = t1 * t->tdat[0]->norm[2] + t2 * t->tdat[1]->norm[2] + t3 * t->tdat[2]->norm[2];

		++tp->overdraw;

		x1 -= 1;
	}
}

static void
rasterize_bot_flat_tri(struct terrain *terra, const struct tri_vertex_data *t,
	const struct pointf *v1, const struct pointf *v2, const struct pointf *v3)
{
	/* return; */
	if (signed_area(v1, v2, v3) < 0) {
		const struct pointf *tmp = v3;
		v3 = v2;
		v2 = tmp;
	}

	double invslope1 = v1->x != v2->x ? (v2->x - v1->x) / (v2->y - v1->y) : 0.0,
	       invslope2 = v1->x != v3->x ? (v3->x - v1->x) / (v3->y - v1->y) : 0.0,
	       curx1 = v1->x,
	       curx2 = v1->x;

	for (float y = v1->y; y > v2->y; y--) {
		draw_line(terra, t, curx1, y, curx2, y);
		curx1 -= invslope1;
		curx2 -= invslope2;
	}
}

static void
rasterize_top_flat_tri(struct terrain *terra, const struct tri_vertex_data *t,
	const struct pointf *v1, const struct pointf *v2, const struct pointf *v3)
{
	/* return; */
	if (signed_area(v1, v2, v3) < 0) {
		const struct pointf *tmp = v1;
		v1 = v2;
		v2 = tmp;
	}

	double invslope1 = v3->x != v1->x ? (v3->x - v1->x) / (v3->y - v1->y) : 0.0,
	       invslope2 = v3->x != v2->x ? (v3->x - v2->x) / (v3->y - v2->y) : 0.0,
	       curx1 = v3->x,
	       curx2 = v3->x;

	for (float y = v3->y; y <= v1->y; y++) {
		draw_line(terra, t, curx1, y, curx2, y);
		curx1 += invslope1;
		curx2 += invslope2;
	}
}

static void
rasterize_tri(struct tg_tri *t, struct terrain *terra)
{
	const struct pointf *v3, *v2, *v1;
	sort_vertices(t->a, t->b, t->c, &v3, &v2, &v1);

	struct terrain_data *tdat[] = {
		hdarr_get(terra->tdat, t->a),
		hdarr_get(terra->tdat, t->b),
		hdarr_get(terra->tdat, t->c)
	};

	assert(tdat[0] && tdat[1] && tdat[2]);

	struct tri_vertex_data tvd = {
		.points = { t->a, t->b, t->c },
		.tdat = { tdat[0], tdat[1], tdat[2] },
	};

	if (v2->y == v3->y) {
		rasterize_bot_flat_tri(terra, &tvd, v1, v2, v3);
	} else if (v1->y == v2->y) {
		rasterize_top_flat_tri(terra, &tvd, v1, v2, v3);
	} else {
		/* general case - split the triangle in a topflat and bottom-flat one */
		struct pointf v4 = {
			(int)(v1->x + ((float)(v2->y - v1->y) / (float)(v3->y - v1->y)) * (v3->x - v1->x)),
			v2->y,
		};

		rasterize_bot_flat_tri(terra, &tvd, v1, v2, &v4);
		rasterize_top_flat_tri(terra, &tvd, v2, &v4, v3);
	}
}

static void
rasterize_terrain(struct trigraph *tg, struct terrain *terra)
{
	uint32_t i;
	for (i = 0; i < hdarr_len(tg->tris); ++i) {
		struct tg_tri *t = darr_get(hdarr_darr(tg->tris), i);

		struct terrain_data *tdat[] = {
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

		rasterize_tri(t, terra);
	}

	for (i = 0; i < hdarr_len(terra->tdat); ++i) {
		struct terrain_data *tdat = darr_get(hdarr_darr(terra->tdat), i);

		vec4_normalize(tdat->norm);
	}

	for (i = 0; i < hdarr_len(tg->tris); ++i) {
		struct tg_tri *t = darr_get(hdarr_darr(tg->tris), i);
		/* enum tile tt = rand_uniform(5); */

		rasterize_tri(t, terra);
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

					if (tp->elev < -5) {
						ck->tiles[rx][ry] = tile_deep_water;
					} else if (tp->elev < 0) {
						ck->tiles[rx][ry] = tile_water;
					} else if (tp->elev < 3) {
						ck->tiles[rx][ry] = tile_wetland;
					} else if (tp->elev < 30) {
						if (tp->waterstop > 20) {
							ck->tiles[rx][ry] = rand_chance(3) ? tile_forest_old : tile_forest;
						} else {
							ck->tiles[rx][ry] = tile_plain;
						}
					} else if (tp->elev < 40) {
						ck->tiles[rx][ry] = tile_mountain;
					} else {
						ck->tiles[rx][ry] = tile_peak;
					}

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
		.tdat = hdarr_init(2048, sizeof(struct pointf *), sizeof(struct terrain_data), NULL),
		.fault_points = darr_init(sizeof(struct pointf *)),
		.heightmap = calloc(opts->height * opts->width, sizeof(struct terrain_pixel)),
	};
	uint32_t i;

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

	L("adding noise");
	add_noise(&terra);

	L("simulating erosion");
	for (i = 0; i < terra.opts.raindrops; ++i) {
		trace_raindrop(&terra, rand_uniform(terra.width), rand_uniform(terra.height));
	}

	L("writing chunks");
	write_chunks(chunks, &terra);
}
