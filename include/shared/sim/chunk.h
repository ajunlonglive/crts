#ifndef __CHUNK_H
#define __CHUNK_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "shared/math/geom.h"
#include "shared/types/hdarr.h"

#define TILE_MAX 4

enum tile {
	tile_sand,
	tile_plain,
	tile_forest,
	tile_mountain,
	tile_peak,
	tile_bldg,
	tile_count,
};

#define CHUNK_SIZE 16

struct chunk {
	struct point pos;
	enum tile tiles[CHUNK_SIZE][CHUNK_SIZE];

	bool empty;

#ifdef CRTS_SERVER
	uint8_t harvested[CHUNK_SIZE][CHUNK_SIZE];
#endif
};

struct chunks {
	struct hdarr *hd;
};

void chunk_init(struct chunk **c);
void chunks_init(struct chunks **cnks);
struct point nearest_chunk(const struct point *p);
#endif
