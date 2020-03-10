#ifndef __CHUNK_H
#define __CHUNK_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "shared/math/geom.h"
#include "shared/types/hash.h"
#include "shared/types/hdarr.h"

#define TILE_MAX 6

enum tile {
	tile_deep_water,
	tile_water,
	tile_sand,
	tile_plain,
	tile_forest,
	tile_mountain,
	tile_peak,
/* Tiles past this will not be randomly generated */
	tile_dirt,
	tile_forest_young,
	tile_forest_old,

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
	size_t last_touched;
#endif
};

struct chunks {
	struct hdarr *hd;

#ifdef CRTS_SERVER
	struct hash *repathfind;
#endif

	size_t chunk_date;
};

void chunk_init(struct chunk **c);
void chunks_init(struct chunks **cnks);
struct point nearest_chunk(const struct point *p);
void chunks_destroy(struct chunks *cnks);
#endif
