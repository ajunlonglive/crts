#include "posix.h"

#include <math.h>

#include "shared/math/geom.h"
#include "shared/types/geom.h"
#include "shared/util/log.h"

int
points_equal(const struct point *a, const struct point *b)
{
	return a->x == b->x && a->y == b->y;
}

int
points_adjacent(const struct point *a, const struct point *b)
{
	return (a->x == b->x &&
		(a->y == b->y - 1 || a->y == b->y + 1)) ||
	       (a->y == b->y &&
		(a->x == b->x - 1 || a->x == b->x + 1));

}

int
point_in_circle(const struct point *p, const struct circle *c)
{
	int a, b;

	a = p->x - c->center.x;
	b = p->y - c->center.y;

	return (a * a) + (b * b) < c->r * c->r;
}

int
point_in_rect(const struct point *p, const struct rectangle *r)
{
	return p->x >= r->pos.x && p->x < r->pos.x + r->width &&
	       p->y >= r->pos.y && p->y < r->pos.y + r->height;
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

int
square_dist(const struct point *a, const struct point *b)
{
	int x = (a->x - b->x), y = (a->y - b->y);

	return x * x + y * y;
}

uint32_t
rect_area(const struct rectangle *rect)
{
	return rect->width * rect->height;
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
