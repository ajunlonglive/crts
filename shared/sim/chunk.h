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
	trav_ne = 0x01,
	trav_nw = 0x02,
	trav_sw = 0x04,
	trav_se = 0x08,
	trav_ns = 0x0f,
	trav_ew = 0xf0,
	trav_al = 0xff,
	trav_no = 0x00,
};

void chunk_init(struct chunk **c);
struct point nearest_chunk(struct point *p);
#endif
