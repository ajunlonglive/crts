#ifndef __CHUNK_H
#define __CHUNK_H

#include "geom.h"

enum tile {
	tile_empty,
};

#define CHUNK_SIZE 64
struct chunk {
	struct point pos;
	enum tile tiles[CHUNK_SIZE][CHUNK_SIZE];
};

void chunk_init(struct chunk **c, const struct point *p);
#endif
