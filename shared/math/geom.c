#include "types/geom.h"
#include "math/geom.h"
#include "util/log.h"

int point_in_circle(const struct point *p, const struct circle *c)
{
	int a, b;

	a = p->x - c->center.x;
	b = p->y - c->center.y;

	return (a * a) + (b * b) < c->r * c->r;
}

int point_in_rect(const struct point *p, const struct rectangle *r)
{
	return p->x >= r->pos.x && p->x <= r->pos.x + r->width &&
	       p->y >= r->pos.y && p->y <= r->pos.y + r->height;
}

void pathfind(struct point *pos, struct point *dest)
{
	int dx, dy, adx, ady, sdx = 1, sdy = 1;

	dx = dest->x - pos->x;
	dy = dest->y - pos->y;
	adx = dx > 0 ? dx : dx * (sdx = -1);
	ady = dy > 0 ? dy : dy * (sdy = -1);

	if (adx > 0 && adx > ady)
		pos->x += sdx;
	else if (ady > 0)
		pos->y += sdy;
}

int distance_point_to_circle(const struct point *p, const struct circle *c)
{
	int a, b;

	a = c->center.x - p->x;
	b = c->center.y - p->y;

	return (a * a) + (b * b) - (c->r * c->r);
}

int dot(const struct point a, const struct point b)
{
	return a.x * b.x + a.y * b.y;
}

struct point point_sub(const struct point *a, const struct point *b)
{
	struct point p = {
		a->x - b->x,
		a->y - b->y
	};

	return p;
}

struct point point_add(const struct point *a, const struct point *b)
{
	struct point p = {
		a->x + b->x,
		a->y + b->y
	};

	return p;
}
