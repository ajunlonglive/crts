#ifndef __CHUNK_H
#define __CHUNK_H

#include "geom.h"

enum tile {
	tile_empty,
	tile_full,
	tile_a,
	tile_b,
	tile_c
};

#define CHUNK_SIZE 16
struct chunk {
	struct {
		struct point terra;
	} noise;

	struct point pos;
	enum tile tiles[CHUNK_SIZE][CHUNK_SIZE];

	int empty;
};

void chunk_init(struct chunk **c);
#endif
