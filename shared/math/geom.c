#include "posix.h"

#include <math.h>
#include <string.h>

#include "shared/math/geom.h"
#include "shared/types/geom.h"
#include "shared/util/log.h"

bool
points_equal(const struct point *a, const struct point *b)
{
	return a->x == b->x && a->y == b->y;
}

bool
points_adjacent(const struct point *a, const struct point *b)
{
	return (a->x == b->x &&
		(a->y == b->y - 1 || a->y == b->y + 1)) ||
	       (a->y == b->y &&
		(a->x == b->x - 1 || a->x == b->x + 1));

}

bool
point_in_circle(const struct point *p, const struct circle *c)
{
	int32_t a, b;

	a = p->x - c->center.x;
	b = p->y - c->center.y;

	return (uint32_t)(a * a) + (uint32_t)(b * b) < c->r * c->r;
}

void
make_rect(const struct pointf *p, float h, float w, struct rect *r)
{
	r->p[0] = (struct pointf){ p->x, p->y };
	r->p[1] = (struct pointf){ p->x, p->y + h };
	r->p[2] = (struct pointf){ p->x + w, p->y + h };
	r->p[3] = (struct pointf){ p->x + w, p->y };
	r->h = h;
	r->w = w;
}

void
translate_rect(struct rect *r, float dx, float dy)
{
	uint32_t i;
	for (i = 0; i < 4; ++i) {
		r->p[i].x += dx;
		r->p[i].y += dy;
	}
}

void
pointf_relative_to_rect(const struct rect *r, float x, float y, struct pointf *res)
{
	x /= r->w;
	y /= r->h;

	struct pointf down = {
		(r->p[3].x - r->p[0].x) * y,
		(r->p[3].y - r->p[0].y) * y,
	};

	struct pointf right = {
		(r->p[1].x - r->p[0].x) * x,
		(r->p[1].y - r->p[0].y) * x,
	};

	res->x = r->p[0].x + right.x + down.x;
	res->y = r->p[0].y + right.y + down.y;
}

void
resize_rect(struct rect *r, float new_width, float new_height)
{
	struct pointf new_points[3] = { 0 };
	pointf_relative_to_rect(r, 0, new_height, &new_points[0]);
	pointf_relative_to_rect(r, new_width, new_height, &new_points[1]);
	pointf_relative_to_rect(r, new_width, 0, &new_points[2]);
	r->p[1] = new_points[0];
	r->p[2] = new_points[1];
	r->p[3] = new_points[2];
	r->h = new_height;
	r->w = new_width;
}

void
containing_axis_aligned_rect(struct rect *r, struct rect *res)
{
	uint32_t i;
	float minx = INFINITY, maxx = -INFINITY,
	      miny = INFINITY, maxy = -INFINITY;

	for (i = 0; i < 4; ++i) {
		if (r->p[i].x > maxx) {
			maxx = r->p[i].x;
		}

		if (r->p[i].x < minx) {
			minx = r->p[i].x;
		}

		if (r->p[i].y > maxy) {
			maxy = r->p[i].y;
		}

		if (r->p[i].y < miny) {
			miny = r->p[i].y;
		}
	}

	float width = maxx - minx, height = maxy - miny;
	make_rect(&(struct pointf){ minx, miny }, height, width, res);
}

bool
point_in_rect(const struct point *p, const struct rect *r)
{
	struct pointf pf = { p->x, p->y };
	return pointf_in_polygon(&pf, r->p, 4);
}

static float
dotf(const struct pointf *a, const struct pointf *b)
{
	return a->x * b->x + a->y * b->y;
}

// Calculate the projection of a polygon on an axis
// and returns it as a [min, max] interval
static void
polygon_project(const struct pointf *axis, const struct pointf *p, uint32_t plen, float *min, float *max)
{
	// To project a point on an axis use the dot product
	float dp = dotf(axis, &p[0]);
	*min = dp;
	*max = dp;

	uint32_t i;
	for (i = 1; i < plen; i++) {
		dp = dotf(axis, &p[i]);
		if (dp < *min) {
			*min = dp;
		} else if (dp > *max) {
			*max = dp;
		}
	}
}

static float
interval_distance(float min1, float max1, float min2, float max2)
{
	if (min1 < min2) {
		return min2 - max1;
	} else {
		return min1 - max2;
	}
}

static bool
polygon_intersect_half(const struct pointf *p1, uint32_t plen1, const struct pointf *p2, uint32_t plen2)
{
	uint32_t i;
	for (i = 0; i < plen1; ++i) {
		struct pointf axis = { -p1[i].y, p1[i].x };
		float mag = sqrtf(axis.x * axis.x + axis.y * axis.y);
		axis.x /= mag;
		axis.y /= mag;

		float min1 = 0, max1 = 0, min2 = 0, max2 = 0;
		polygon_project(&axis, p1, plen1, &min1, &max1);
		polygon_project(&axis, p2, plen2, &min2, &max2);

		if (interval_distance(min1, max1, min2, max2) > 0) {
			return false;
		}
	}

	return true;
}

bool
polygon_intersects(const struct pointf *p1, uint32_t plen1, const struct pointf *p2, uint32_t plen2)
{
	return polygon_intersect_half(p1, plen1, p2, plen2)
	       && polygon_intersect_half(p2, plen2, p1, plen1);
}

float
rect_area(const struct rect *r)
{
	return polygon_area(r->p, 4);
}

float
polygon_area(const struct pointf *p, uint32_t plen)
{
	float sum = 0.0f;
	uint32_t i;
	for (i = 0; i < plen - 1; ++i) {
		sum += (p[i].y + p[i + 1].y) * (p[i].x - p[i + 1].x);
	}
	sum += (p[i].y + p[0].y) * (p[i].x - p[0].x);

	return sum / 2.0f;
}

bool
pointf_in_polygon(const struct pointf *p, const struct pointf *poly, uint32_t plen)
{
	const struct pointf *p1, *p2;
	float dist;
	uint8_t i;
	for (i = 0; i < plen; ++i) {
		p1 = &poly[i], p2 = &poly[(i + 1) % plen];

		dist = (p2->x - p1->x) * (p->y - p1->y) - (p->x - p1->x) * (p2->y - p1->y);
		if (dist <= 0) {
			return false;
		}
	}

	return true;
}

int
distance_point_to_circle(const struct point *p, const struct circle *c)
{
	int a, b;

	a = c->center.x - p->x;
	b = c->center.y - p->y;

	return (a * a) + (b * b) - (c->r * c->r);
}

static int
roundto(int i, int nearest)
{
	int m = i % nearest;

	return m >= 0 ? i - m : i - (nearest + m);
}

struct point
point_mod(const struct point *p, int operand)
{
	struct point q = {
		roundto(p->x, operand),
		roundto(p->y, operand)
	};

	return q;
}


int
dot(const struct point a, const struct point b)
{
	return a.x * b.x + a.y * b.y;
}

struct point
point_sub(const struct point *a, const struct point *b)
{
	struct point p = {
		a->x - b->x,
		a->y - b->y
	};

	return p;
}

struct point
point_add(const struct point *a, const struct point *b)
{
	struct point p = {
		a->x + b->x,
		a->y + b->y
	};

	return p;
}

uint32_t
square_dist(const struct point *a, const struct point *b)
{
	int32_t x = (a->x - b->x), y = (a->y - b->y);

	return x * x + y * y;
}

float
fsqdist(const struct pointf *p, const struct pointf *q)
{
	float x = p->x - q->x,
	      y = p->y - q->y;

	return x * x + y * y;
}

void
make_line(struct pointf *p, struct pointf *q, line l)
{
	l[0] = q->y - p->y;
	l[1] = p->x - q->x;
	l[2] = l[0] * p->x + l[1] * p->y;
}

void
make_perpendicular_bisector(struct pointf *p, struct pointf *q, line l)
{
	make_line(p, q, l);

	float mx = (p->x + q->x) / 2.0f,
	      my = (p->y + q->y) / 2.0f;

	l[2] = -l[1] * mx + l[0] * my;
	float tmp = l[0];
	l[0] = -l[1];
	l[1] = tmp;
}

bool
intersection_of(line l1, line l2, struct pointf *p)
{
	float d;

	if ((d = l1[0] * l2[1] - l2[0] * l1[1]) == 0.0f) {
		return false;
	}

	p->x = (l2[1] * l1[2] - l1[1] * l2[2]) / d;
	p->y = (l1[0] * l2[2] - l2[0] * l1[2]) / d;
	return true;
}

float
signed_area(const struct pointf *v0, const struct pointf *v1, const struct pointf *v2)
{
	return ((v1->x - v0->x) * (v2->y - v0->y) - (v2->x - v0->x) * (v1->y - v0->y)) / 2.0f;
}

float
nearest_neighbour(float a, float b, float c, float d, float x, float y)
{
	float diffx = x - floorf(x), diffy = y - floorf(y);

	return a * (1 - diffx) * (1 - diffy)
	       + b * diffx * (1 - diffy)
	       + c *  (1 - diffx) * diffy
	       + d * diffx * diffy;
}

void
rotate_pointf(struct pointf *p, const struct pointf *axis, float angle)
{
	double s = sin(angle);
	double c = cos(angle);

	p->x -= axis->x;
	p->y -= axis->y;

	p->x = (double)(p->x * c - p->y * s) + axis->x;
	p->y = (double)(p->x * s + p->y * c) + axis->y;
}

void
rotate_poly(struct pointf *p, uint32_t plen, const struct pointf *axis, float angle)
{
	uint32_t i;
	float s = sinf(angle);
	float c = cosf(angle);

	for (i = 0; i < plen; ++i) {
		p[i].x -= axis->x;
		p[i].y -= axis->y;

		p[i].x = p[i].x * c - p[i].y * s + axis->x;
		p[i].y = p[i].x * s + p[i].y * c + axis->y;
	}
}

void
rotate_rect(struct rect *r, const struct pointf *axis, float angle)
{
	rotate_poly(r->p, 4, axis, angle);
}

void
make_rotated_rect(const struct pointf *c, float h, float w, float a, struct rect *r)
{
	r->h = h;
	r->w = w;
	h /= 2.0f;
	w /= 2.0f;

	float sina = sinf(a);
	float cosa = cosf(a);

	struct pointf right = { w * sina, w * cosa },
		      up = { h * cosa, -h * sina };

	r->p[0] = (struct pointf){ c->x + right.x - up.x, c->y + right.y - up.y };
	r->p[1] = (struct pointf){ c->x - right.x - up.x, c->y - right.y - up.y };
	r->p[2] = (struct pointf){ c->x - right.x + up.x, c->y - right.y + up.y };
	r->p[3] = (struct pointf){ c->x + right.x + up.x, c->y + right.y + up.y };
}
