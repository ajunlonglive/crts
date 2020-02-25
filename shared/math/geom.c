#include "shared/math/geom.h"
#include "shared/types/geom.h"
#include "shared/util/log.h"

int
points_equal(const struct point *a, const struct point *b)
{
	return a->x == b->x && a->y == b->y;
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
