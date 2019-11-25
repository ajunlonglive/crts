#include "geom.h"

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
	adx = dx > 0 ? dx : dx * (sdx = -1);

	if (adx > 0) {
		pos->x += sdx;
		return;
	}

	dy = dest->y - pos->y;
	ady = dy > 0 ? dy : dy * (sdy = -1);

	if (ady > 0)
		pos->y += sdy;
}
