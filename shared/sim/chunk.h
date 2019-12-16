#ifndef __CHUNK_H
#define __CHUNK_H

#include "math/geom.h"

#define TILE_MAX 4
enum tile {
	tile_sand     = 0,
	tile_plain    = 1,
	tile_forest   = 2,
	tile_mountain = 3,
	tile_peak     = 4
};

#define CHUNK_SIZE 16
struct chunk {
	struct point pos;
	enum tile tiles[CHUNK_SIZE][CHUNK_SIZE];

	int empty;
	int trav;
};

enum traversability {
	trav_n  = 0x1,
	trav_s  = 0x2,
	trav_e  = 0x4,
	trav_w  = 0x8,
	trav_al = 0xf,
	trav_no = 0x0,
};

void chunk_init(struct chunk **c);
struct point nearest_chunk(const struct point *p);
#endif
