#ifndef __GEOM_H
#define __GEOM_H
#include <stdbool.h>

#include "shared/types/geom.h"

#define PI 3.1415927f
#define R2D(r) (180.0 * (r) / PI)

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
uint32_t rect_area(const struct rectangle *rect);

float fsqdist(const struct pointf *p, const struct pointf *q);
void make_line(struct pointf *p, struct pointf *q, line l);
float signed_area(const struct pointf *v0, const struct pointf *v1, const struct pointf *v2);
bool intersection_of(line l1, line l2, struct pointf *p);
void make_perpendicular_bisector(struct pointf *p, struct pointf *q, line l);
#endif
