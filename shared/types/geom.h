#ifndef __TYPES_GEOM_H
#define __TYPES_GEOM_H
struct point {
	int x;
	int y;
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
#endif
