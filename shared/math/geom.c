#include "shared/math/geom.h"
#include "shared/types/geom.h"
#include "shared/util/log.h"

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
