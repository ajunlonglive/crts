#ifndef __GEOM_H
#define __GEOM_H

struct point {
	int x;
	int y;
};

struct point3 {
	int x;
	int y;
	int z;
};

struct circle {
	struct point center;
	int r;
};

struct rectangle {
	struct point pos; // upper left corner
	int width;
	int height;
};

int point_in_circle(const struct point *p, const struct circle *c);
int point_in_rect(const struct point *p, const struct rectangle *r);
void pathfind(struct point *pos, struct point *dest);
int distance_point_to_circle(const struct point *p, const struct circle *c);
int dot(const struct point a, const struct point b);
struct point point_sub(const struct point *a, const struct point *b);
#endif
