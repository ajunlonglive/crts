#include "posix.h"

#include <assert.h>
#include <math.h>
#include <stdlib.h>

#include "shared/math/geom.h"
#include "shared/math/rand.h"
#include "shared/math/trigraph.h"
#include "shared/types/darr.h"
#include "shared/types/hdarr.h"

struct triangulation {
	struct darr hull, tmphull, heap;
	struct pointf cc;
	struct trigraph *tg;
};

struct triangulation *qsort_ctx;
static int
compare_dist_from_cc(const void *a, const void *b)
{
	const struct pointf *p = darr_get(&qsort_ctx->tg->points, *(uint32_t *)a),
			    *q = darr_get(&qsort_ctx->tg->points, *(uint32_t *)b);

	float d1 = fsqdist(p, &qsort_ctx->cc),
	      d2 = fsqdist(q, &qsort_ctx->cc);

	return d1 < d2 ? 1 : (d1 > d2 ? -1 : 0);
}

static void
push_tri(struct trigraph *tg,
	uint32_t a, uint32_t b, uint32_t c)
{
	tg_get_tri(tg, darr_get(&tg->points, a), darr_get(&tg->points, b),
		darr_get(&tg->points, c));
}

static struct pointf *
heap_point(struct triangulation *g, uint32_t heapindex)
{
	return darr_get(&g->tg->points,
		*(uint32_t *)darr_get(&g->heap, heapindex));
}

static struct pointf *
hull_point(struct triangulation *g, uint32_t hullindex)
{
	return darr_get(&g->tg->points,
		*(uint32_t *)darr_get(&g->hull, hullindex));
}

static uint32_t
heap_pop(struct triangulation *g, uint32_t heapindex)
{
	uint32_t r = *(uint32_t *)darr_get(&g->heap, heapindex);

	darr_del(&g->heap, heapindex);

	return r;
}

static void
triangulate(struct triangulation *g)
{
	uint32_t i;
	uint32_t tri[3] = { 0 };
	int32_t mindex;
	struct pointf *trip[3] = { 0 };

	/* pick initial seed point randomly */
	tri[0] = heap_pop(g, rand_uniform(darr_len(&g->heap)));
	trip[0] = darr_get(&g->tg->points, tri[0]);

	/* find 2nd seed point, the closest point to initial point */
	float dist, mindist = INFINITY;
	mindex = -1;

	for (i = 0; i < darr_len(&g->heap); ++i) {
		struct pointf *p = heap_point(g, i);

		if ((dist = fsqdist(trip[0], p)) < mindist) {
			mindex = i;
			mindist = dist;
		}
	}

	assert(mindex >= 0);

	tri[1] = heap_pop(g, mindex);
	trip[1] = darr_get(&g->tg->points, tri[1]);
	line b1;
	make_perpendicular_bisector(trip[0], trip[1], b1);

	/* find 3rd seed point, which when combined with the first two points
	 * into a triangle has the smallest circumcircle */
	mindist = INFINITY;
	struct pointf cc;
	mindex = -1;

	for (i = 0; i < darr_len(&g->heap); ++i) {
		struct pointf *p = heap_point(g, i);

		line b2;
		make_perpendicular_bisector(trip[0], p, b2);

		if (intersection_of(b1, b2, &cc)
		    && ((dist = fsqdist(trip[0], &cc)) < mindist)) {
			mindist = dist;
			g->cc = cc;
			mindex = i;
		}
	}

	assert(mindex >= 0);

	tri[2] = heap_pop(g, mindex);
	trip[2] = darr_get(&g->tg->points, tri[2]);

	/* sort vertices into counter-clockwise order */
	if (signed_area(trip[0], trip[1], trip[2]) > 0) {
		/* already counter-clockwise */
		push_tri(g->tg, tri[0], tri[1], tri[2]);
		darr_push(&g->hull, &tri[0]);
		darr_push(&g->hull, &tri[1]);
		darr_push(&g->hull, &tri[2]);
	} else {
		/* clockwise, reverse order */
		push_tri(g->tg, tri[0], tri[2], tri[1]);
		darr_push(&g->hull, &tri[0]);
		darr_push(&g->hull, &tri[2]);
		darr_push(&g->hull, &tri[1]);
	}

	/* sort remaning heap by distance from initial triangle circumcircle */
	qsort_ctx = g;
	qsort(darr_raw_memory(&g->heap), darr_len(&g->heap),
		darr_item_size(&g->heap), compare_dist_from_cc);

	while (darr_len(&g->heap)) {
		uint32_t cur = heap_pop(g, darr_len(&g->heap) - 1);
		struct pointf *p = darr_get(&g->tg->points, cur);
		int32_t first = -1, start = -1;
		uint32_t lasti;

		darr_clear(&g->tmphull);

		for (i = 0; (int32_t)i != start; i = (i + 1) % darr_len(&g->hull)) {
			lasti = i ? i - 1 : darr_len(&g->hull) - 1;

			if (signed_area(hull_point(g, i), hull_point(g, lasti), p) <= 0) {
				darr_push(&g->tmphull, darr_get(&g->hull, lasti));

				if (start < 0) {
					start = i;
				}
				continue;
			} else if (start < 0) {
				continue;
			}

			uint32_t b = *(uint32_t *)darr_get(&g->hull, lasti),
				 c = *(uint32_t *)darr_get(&g->hull, i);

			if (first == -1) {
				first = b;
				darr_push(&g->tmphull, &b);
				darr_push(&g->tmphull, &cur);
			}

			push_tri(g->tg, cur, b, c);
		}

		struct darr tmp = g->hull;
		g->hull = g->tmphull;
		g->tmphull = tmp;
	}
}

static bool
needs_flipping(struct trigraph *g, const struct tg_edge *e)
{
	const struct tg_tri *adja = tg_get_trik(g, e->adja),
			    *adjb = tg_get_trik(g, e->adjb);

	return adjb
	       && tg_point_angle(adja, e->a) + tg_point_angle(adjb, e->a) < PI
	       && tg_point_angle(adja, e->b) + tg_point_angle(adjb, e->b) < PI
	       && tg_opposite_angle(adja, e) + tg_opposite_angle(adjb, e) > PI;
}

static void
flip(struct trigraph *g, const struct tg_edge *e)
{
	const struct tg_tri *tp[] = { tg_get_trik(g, e->adja), tg_get_trik(g, e->adjb) };
	struct tg_tri t[] = { *tp[0], *tp[1] };
	const struct pointf *fp[4];

	assert(e->adja != NULL && e->adjb != NULL);
	assert(e->a <= e->b);

	if (tg_edges_eql(e, t[0].ab)) {
		fp[0] = t[0].c;
		fp[1] = t[0].a;
		fp[2] = t[0].b;
	} else if (tg_edges_eql(e, t[0].bc)) {
		fp[0] = t[0].a;
		fp[1] = t[0].b;
		fp[2] = t[0].c;
	} else if (tg_edges_eql(e, t[0].ac)) {
		fp[0] = t[0].b;
		fp[2] = t[0].c;
		fp[1] = t[0].a;
	} else {
		assert(false);
		return;
	}

	if (tg_edges_eql(e, t[1].ab)) {
		fp[3] = t[1].c;
	} else if (tg_edges_eql(e, t[1].bc)) {
		fp[3] = t[1].a;
	} else if (tg_edges_eql(e, t[1].ac)) {
		fp[3] = t[1].b;
	} else {
		assert(false);
		return;
	}

	tg_del_tri(g, tp[0]);
	tg_del_tri(g, tp[1]);

	tg_get_tri(g, fp[0], fp[3], fp[2]);
	tg_get_tri(g, fp[1], fp[3], fp[0]);
}

static void
flip_edges(struct trigraph *g)
{
	uint32_t i, j;
	bool found;
	uint32_t flips = 0;

	do {
		found = false;

		for (i = 0; i < hdarr_len(&g->tris); ++i) {
			const struct tg_tri *t = darr_get(&g->tris.darr, i);
			const tg_edgekey *ek[] = { &t->ab, &t->bc, &t->ac };

			for (j = 0; j < 3; ++j) {
				const struct tg_edge *es = tg_get_edgek(g, *ek[j]);
				assert(es);

				if (needs_flipping(g, es)) {
					found = true;
					flip(g, es);
					++flips;

				}
			}
		}
	} while (found);
}

void
delaunay(struct trigraph *tg)
{
	struct triangulation g = { 0 };

	darr_init(&g.heap, sizeof(uint32_t));
	darr_init(&g.hull, sizeof(uint32_t));
	darr_init(&g.tmphull, sizeof(uint32_t));
	g.tg = tg;

	uint32_t i;
	for (i = 0; i < darr_len(&tg->points); ++i) {
		darr_push(&g.heap, &i);
	}

	assert(darr_len(&tg->points) >= 3);

	triangulate(&g);

	flip_edges(tg);
}
