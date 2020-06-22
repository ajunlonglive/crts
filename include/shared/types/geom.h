#ifndef SHARED_TYPES_GEOM_H
#define SHARED_TYPES_GEOM_H

#include <stdint.h>

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
