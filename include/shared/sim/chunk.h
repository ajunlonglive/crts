#ifndef __CHUNK_H
#define __CHUNK_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "shared/math/geom.h"
#include "shared/types/hdarr.h"

#ifdef CRTS_SERVER
#include "shared/types/hash.h"
#endif

#define TILE_MAX 6

enum tile {
	tile_deep_water,
	tile_water,
	tile_wetland,
	tile_plain,
	tile_forest,
	tile_mountain,
	tile_peak,
/* Tiles past this will not be randomly generated */
	tile_dirt,
	tile_forest_young,
	tile_forest_old,
	tile_wetland_forest_young,
	tile_wetland_forest,
	tile_wetland_forest_old,
	tile_coral,

	tile_wood,
	tile_stone,
	tile_wood_floor,
	tile_rock_floor,
	tile_shrine,
	tile_farmland_empty,
	tile_farmland_done,
	tile_burning,
	tile_burnt,

	tile_count,
};

_Static_assert(tile_count == 23, "update tile count in shader");

#define CHUNK_SIZE 16

struct chunk {
	struct point pos;
	uint32_t tiles[CHUNK_SIZE][CHUNK_SIZE];

#ifdef CRTS_SERVER
	size_t last_touched;
	uint16_t harvested[CHUNK_SIZE][CHUNK_SIZE];
	bool touched_this_tick;
#endif
	bool empty;
};

struct chunks {
	struct hdarr *hd;

#ifdef CRTS_SERVER
	struct hash *functional_tiles;
#endif

	size_t chunk_date;
};

/* TODO: replace hash value type with uint64_t so we always know how many bits
 * it has
 */
_Static_assert(sizeof(size_t) == 8, "wrong size size_t");

#ifdef CRTS_SERVER
union functional_tile {
	size_t val;

	struct {
		uint16_t type;
		uint16_t motivator;
		uint16_t age;
		uint16_t _pad;
	} ft;
};
#endif

void chunk_init(struct chunk **c);
void chunks_init(struct chunks **cnks);
struct point nearest_chunk(const struct point *p);
void chunks_destroy(struct chunks *cnks);
#endif
