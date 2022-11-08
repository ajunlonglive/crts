#ifndef SHARED_TYPES_GEOM_H
#define SHARED_TYPES_GEOM_H

#include <stdint.h>

struct point {
	int32_t x;
	int32_t y;
};

struct point3d {
	int32_t x, y, z;
};

struct circle {
	struct point center;
	uint32_t r;
};

struct pointf {
	float x, y;
};

struct rect {
	struct pointf p[4];
	float w, h;
};

typedef float line[3];
#endif
