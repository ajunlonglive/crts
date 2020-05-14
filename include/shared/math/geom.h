#ifndef __GEOM_H
#define __GEOM_H
#include "shared/types/geom.h"

#define PI 3.1415927f

int points_equal(const struct point *a, const struct point *b);
int points_adjacent(const struct point *a, const struct point *b);
int point_in_circle(const struct point *p, const struct circle *c);
int point_in_rect(const struct point *p, const struct rectangle *r);
struct point point_mod(const struct point *p, int operand);
int distance_point_to_circle(const struct point *p, const struct circle *c);
int dot(const struct point a, const struct point b);
struct point point_sub(const struct point *a, const struct point *b);
struct point point_add(const struct point *a, const struct point *b);
int square_dist(const struct point *a, const struct point *b);
#endif
