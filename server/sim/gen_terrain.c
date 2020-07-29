#include <math.h>

#include <stdlib.h>

#include "server/sim/gen_terrain.h"
#include "server/sim/terrain.h"
#include "shared/math/delaunay.h"
#include "shared/math/rand.h"
#include "shared/math/perlin.h"
#include "shared/math/geom.h"
#include "shared/types/darr.h"
#include "shared/util/log.h"

static void
cartesian_to_barycentric(const struct tg_tri *t,
	float x, float y, float *l1, float *l2, float *l3)
{
	assert(signed_area(t->a, t->b, t->c) > 0);

	float x1 = t->a->x, y1 = t->a->y,
	      x2 = t->b->x, y2 = t->b->y,
	      x3 = t->c->x, y3 = t->c->y,
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
};

struct terrain_pixel {
	float elev;
	enum tile t;
	uint32_t overdraw;
};

struct terrain {
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
	float boost = rand_chance(4) ? (20 - rand_uniform(10)) : (rand_uniform(20) + 10);

gen_fault_pass:
	p = first_pass ? oe->a : oe->b;

	uint32_t i = 0;
	while (++i < 1000 && fsqdist(p, &terra->mid) < terra->radius * 0.75) {
		/* for (i = 0; i < 1000; ++i) { */
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
		float angle = 0.0, nextang;

		t = hdarr_get(tg->tris, e->adja);
		while (angle < PI) {
			nextang = tg_point_angle(t, p);

			if (angle + nextang > PI * 1.2 && angle > 0.0) {
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
	float decay = log(darr_len(tg->points)) / log(13000);
	L("decay: %f", decay);

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
			.boost_decay = decay,
		};

		tg_for_each_adjacent_point(tg, td->p, td->faultedge, &ctx, fill_plates_recursor);
	}
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

static void
draw_line(struct terrain *terra, const struct tg_tri *t,
	float x1, float y1, float x2, float y2)
{
	assert(y1 == y2);

	int32_t iy = roundf(y1), ix;
	if (iy < 0 || iy >= terra->height) {
		return;
	}

	/* TODO: if the triangles were all in the right winding order, pretty
	 * sure I wouldn't need to do this */
	if (x1 < x2) {
		float tmp = x2;
		x2 = x1;
		x1 = tmp;
	}

	x1 += 1;

	struct terrain_data *tdat[] = {
		hdarr_get(terra->tdat, t->a),
		hdarr_get(terra->tdat, t->b),
		hdarr_get(terra->tdat, t->c)
	};

	assert(tdat[0] && tdat[1] && tdat[2]);
	assert(tdat[0]->p == t->a && tdat[1]->p == t->b && tdat[2]->p == t->c);
	float h1 = tdat[0]->elev, h2 = tdat[1]->elev, h3 = tdat[2]->elev;

	while (x1 >= x2) {
		ix = roundf(x1);

		if (ix < 0 || ix >= terra->width) {
			x1 -= 1;
			continue;
		}

		float t1 = 0, t2 = 0, t3 = 0;

		cartesian_to_barycentric(t, x1, y1, &t1, &t2, &t3);

		float elev = t1 * h1 + t2 * h2 + t3 * h3;

		uint32_t index = (iy * terra->width) + ix;
		/* L("%f, %f: index: %d", x1, y1, index); */
		assert(index < terra->height * terra->width);

		struct terrain_pixel *tp = &terra->heightmap[index];

		tp->elev = elev;

		++tp->overdraw;
		/* L("elev: %f", elev); */

		/* draw_point(chunks, elev, &p); */
		/* fprintf(stderr, "%f, ", x1); */
		x1 -= 1;
	}
}

static void
rasterize_bot_flat_tri(struct terrain *terra,
	const struct tg_tri *t, const struct pointf *v1, const struct pointf *v2,
	const struct pointf *v3)
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
rasterize_top_flat_tri(struct terrain *terra, const struct tg_tri *t,
	const struct pointf *v1, const struct pointf *v2,
	const struct pointf *v3)
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

	if (v2->y == v3->y) {
		rasterize_bot_flat_tri(terra, t, v1, v2, v3);
	} else if (v1->y == v2->y) {
		rasterize_top_flat_tri(terra, t, v1, v2, v3);
	} else {
		/* general case - split the triangle in a topflat and bottom-flat one */
		struct pointf v4 = {
			(int)(v1->x + ((float)(v2->y - v1->y) / (float)(v3->y - v1->y)) * (v3->x - v1->x)),
			v2->y,
		};

		rasterize_bot_flat_tri(terra, t, v1, v2, &v4);
		rasterize_top_flat_tri(terra, t, v2, &v4, v3);
	}
}

static void
rasterize_terrain(struct trigraph *tg, struct terrain *terra)
{
	uint32_t i;
	for (i = 0; i < hdarr_len(tg->tris); ++i) {
		struct tg_tri *t = darr_get(hdarr_darr(tg->tris), i);
		/* enum tile tt = rand_uniform(5); */

		rasterize_tri(t, terra);
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

					tp->elev += perlin_two(q.x, q.y, 1.0, 1, 0.01, 1.4);

					if (tp->elev < -5) {
						ck->tiles[rx][ry] = tile_deep_water;
					} else if (tp->elev < 0) {
						ck->tiles[rx][ry] = tile_water;
					} else {
						ck->tiles[rx][ry] = tile_plain;
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
gen_terrain(struct chunks *chunks, uint32_t width, uint32_t height, uint32_t points)
{
	struct trigraph tg = { 0 };
	struct pointf corner = { 0.0, 0.0 };
	struct terrain terra = {
		.height = height,
		.width = width,
		.mid = { width * 0.5, height * 0.5 },
		.tdat = hdarr_init(2048, sizeof(struct pointf *), sizeof(struct terrain_data), NULL),
		.fault_points = darr_init(sizeof(struct pointf *)),
		.heightmap = calloc(height * width, sizeof(struct terrain_pixel)),
	};

	terra.radius = fsqdist(&terra.mid, &corner) * 0.5;

	trigraph_init(&tg);
	tg_scatter(&tg, width, height, points, true);

	delaunay(&tg);
	L("tris: %ld", hdarr_len(tg.tris));

	L("seeding terrain");
	seed_terrain_data(&tg, &terra);

	L("generating fault lines");
	uint32_t i;
	for (i = 0; i < 40; ++i) {
		gen_fault(&tg, &terra);
	}

	L("filling tectonic plates");
	fill_plates(&tg, &terra);

	L("rasterizing terrain heightmap");
	rasterize_terrain(&tg, &terra);

	L("writing chunks");
	write_chunks(chunks, &terra);
}
