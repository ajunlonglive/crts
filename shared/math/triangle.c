#include "posix.h"

#include <assert.h>
#include <math.h>
#include <stddef.h>

#include "shared/math/geom.h"
#include "shared/math/triangle.h"
#include "shared/math/trigraph.h"

static void
cartesian_to_barycentric(const struct pointf *a, const struct pointf *b,
	const struct pointf *c, float x, float y, float *l1, float *l2, float *l3)
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
draw_line(float x1, float y1, float x2, float y2, struct tg_tri *t, void *ctx,
	float vd[][3], size_t vd_len, rasterize_tri_callback cb)
{
	uint32_t i;
	int32_t iy = roundf(y1), ix;
	float interp_data[16];

	assert(vd_len < 16);
	assert(y1 == y2);

	if (x1 < x2) {
		float tmp = x2;
		x2 = x1;
		x1 = tmp;
	}

	x1 += 1;

	while (x1 >= x2) {
		ix = roundf(x1);

		float t1 = 0, t2 = 0, t3 = 0;

		cartesian_to_barycentric(t->a, t->b, t->c,
			x1, y1, &t1, &t2, &t3);

		for (i = 0; i < vd_len; ++i) {
			interp_data[i] = t1 * vd[i][0] + t2 * vd[i][1] + t3 * vd[i][2];
		}

		cb(ctx, interp_data, vd_len, ix, iy);

		x1 -= 1;
	}
}

static void
rasterize_bot_flat_tri(void *ctx, float vertex_data[][3], size_t vd_len,
	rasterize_tri_callback cb, struct tg_tri *t,
	const struct pointf *v1, const struct pointf *v2,
	const struct pointf *v3)
{
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
		draw_line(curx1, y, curx2, y, t, ctx, vertex_data, vd_len, cb);
		curx1 -= invslope1;
		curx2 -= invslope2;
	}
}

static void
rasterize_top_flat_tri(void *ctx, float vertex_data[][3], size_t vd_len,
	rasterize_tri_callback cb, struct tg_tri *t,
	const struct pointf *v1, const struct pointf *v2,
	const struct pointf *v3)
{
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
		draw_line(curx1, y, curx2, y, t, ctx, vertex_data, vd_len, cb);
		curx1 += invslope1;
		curx2 += invslope2;
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

void
rasterize_tri(struct tg_tri *t, void *ctx, float vertex_data[][3], size_t vd_len,
	rasterize_tri_callback cb)
{
	const struct pointf *v3, *v2, *v1;
	sort_vertices(t->a, t->b, t->c, &v3, &v2, &v1);

	if (v2->y == v3->y) {
		rasterize_bot_flat_tri(ctx, vertex_data, vd_len, cb, t, v1, v2, v3);
	} else if (v1->y == v2->y) {
		rasterize_top_flat_tri(ctx, vertex_data, vd_len, cb, t, v1, v2, v3);
	} else {
		struct pointf v4 = {
			(int)(v1->x + ((float)(v2->y - v1->y)
				       / (float)(v3->y - v1->y)) * (v3->x - v1->x)),
			v2->y,
		};

		rasterize_bot_flat_tri(ctx, vertex_data, vd_len, cb, t, v1, v2, &v4);
		rasterize_top_flat_tri(ctx, vertex_data, vd_len, cb, t, v2, &v4, v3);
	}
}
