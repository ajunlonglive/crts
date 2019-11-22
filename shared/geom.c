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
	pos->x++; //+= pos->x - dest->x;
	pos->y++; //+= pos->y - dest->y;
}
