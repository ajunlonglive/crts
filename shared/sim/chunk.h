#ifndef __CHUNK_H
#define __CHUNK_H

#include "math/geom.h"

#define TILE_MAX 4
enum tile {
	tile_sand,
	tile_plain,
	tile_forest,
	tile_mountain,
	tile_peak
};

#define CHUNK_SIZE 16
struct chunk {
	struct point pos;
	enum tile tiles[CHUNK_SIZE][CHUNK_SIZE];

	int empty;
};

void chunk_init(struct chunk **c);
struct point nearest_chunk(struct point *p);
#endif
