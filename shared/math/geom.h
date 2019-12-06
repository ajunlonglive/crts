#ifndef __GEOM_H
#define __GEOM_H
#include "types/geom.h"

int point_in_circle(const struct point *p, const struct circle *c);
int point_in_rect(const struct point *p, const struct rectangle *r);
void pathfind(struct point *pos, struct point *dest);
int distance_point_to_circle(const struct point *p, const struct circle *c);
int dot(const struct point a, const struct point b);
struct point point_sub(const struct point *a, const struct point *b);
struct point point_add(const struct point *a, const struct point *b);
#endif
