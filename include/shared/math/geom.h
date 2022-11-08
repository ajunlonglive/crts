#ifndef __GEOM_H
#define __GEOM_H
#include <stdbool.h>

#include "shared/types/geom.h"

#define PI 3.1415927f
#define R2D(r) (180.0 * (r) / PI)
#define D2R(n) (n * PI / 180)

bool points_equal(const struct point *a, const struct point *b);
bool points_adjacent(const struct point *a, const struct point *b);
bool point_in_circle(const struct point *p, const struct circle *c);
struct point point_mod(const struct point *p, int operand);
int32_t distance_point_to_circle(const struct point *p, const struct circle *c);
int32_t dot(const struct point a, const struct point b);
struct point point_sub(const struct point *a, const struct point *b);
struct point point_add(const struct point *a, const struct point *b);
uint32_t square_dist(const struct point *a, const struct point *b);

void make_rect(const struct pointf *p, float h, float w, struct rect *r);
void make_rotated_rect(const struct pointf *c, float h, float w, float a, struct rect *r);
void containing_axis_aligned_rect(struct rect *r, struct rect *res);
void resize_rect(struct rect *r, float new_width, float new_height);
void translate_rect(struct rect *r, float dx, float dy);
void pointf_relative_to_rect(const struct rect *r, float x, float y, struct pointf *res);
bool point_in_rect(const struct point *p, const struct rect *r);
bool pointf_in_polygon(const struct pointf *p, const struct pointf *poly, uint32_t plen);
float rect_area(const struct rect *r);
float polygon_area(const struct pointf *p, uint32_t plen);
bool polygon_intersects(const struct pointf *p1, uint32_t plen1, const struct pointf *p2, uint32_t plen2);

float fsqdist(const struct pointf *p, const struct pointf *q);
void make_line(struct pointf *p, struct pointf *q, line l);
float signed_area(const struct pointf *v0, const struct pointf *v1, const struct pointf *v2);
bool intersection_of(line l1, line l2, struct pointf *p);
void make_perpendicular_bisector(struct pointf *p, struct pointf *q, line l);
float nearest_neighbour(float a, float b, float c, float d, float x, float y);
void rotate_pointf(struct pointf *p, const struct pointf *axis, float angle);
void rotate_rect(struct rect *r, const struct pointf *axis, float angle);
#endif
