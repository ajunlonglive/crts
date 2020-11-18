#ifndef SHARED_SIM_CHUNK_H
#define SHARED_SIM_CHUNK_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "shared/math/geom.h"
#include "shared/types/darr.h"
#include "shared/types/hdarr.h"

#ifdef CRTS_PATHFINDING
#include "shared/pathfind/abstract_graph.h"
#endif

#ifdef CRTS_SERVER
#include "shared/types/hash.h"
#endif

enum tile {
	tile_deep_water,
	tile_water,
	tile_wetland,
	tile_plain,
	tile_forest,
	tile_mountain,
	tile_peak,
	tile_dirt,
	tile_forest_young,
	tile_forest_old,
	tile_wetland_forest_young,
	tile_wetland_forest,
	tile_wetland_forest_old,
	tile_coral,
	tile_stream,

	tile_wood,
	tile_stone,
	tile_wood_floor,
	tile_rock_floor,
	tile_farmland_empty,
	tile_farmland_done,
	tile_burning,
	tile_burnt,
	tile_storehouse,

	tile_count,
};

enum tile_function {
	tfunc_none,
	tfunc_dynamic,
	tfunc_storage,
};

#ifdef CRTS_SERVER
#define STOREHOUSE_SLOTS 4
struct storehouse_storage {
	uint8_t type[STOREHOUSE_SLOTS];
	uint8_t amnt[STOREHOUSE_SLOTS];
	struct point pos;
	uint32_t ent;
	uint16_t owner;
	bool deleted;
};
#endif

#define CHUNK_SIZE 16

_Static_assert(tile_count < 256, "increase type of tiles in chunk");

struct chunk {
	uint8_t tiles[CHUNK_SIZE][CHUNK_SIZE];
	float heights[CHUNK_SIZE][CHUNK_SIZE];

#ifdef CRTS_SERVER
	uint8_t harvested[CHUNK_SIZE][CHUNK_SIZE];
	size_t last_touched;
	bool touched_this_tick;
#endif
	struct point pos;
};

struct chunks {
#ifdef CRTS_PATHFINDING
	struct abstract_graph ag;
#endif

	struct hdarr hd;

#ifdef CRTS_SERVER
	struct darr storehouses;
	struct hash functional_tiles;
	struct hash functional_tiles_buf;
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

void chunks_init(struct chunks *cnks);
struct point nearest_chunk(const struct point *p);
void chunks_destroy(struct chunks *cnks);
struct chunk *get_chunk(struct chunks *cnks, const struct point *p);
struct chunk *get_chunk_at(struct chunks *cnks, const struct point *p);
void set_chunk(struct chunks *cnks, struct chunk *ck);

#ifdef CRTS_SERVER
void touch_chunk(struct chunks *cnks, struct chunk *ck);
#endif
#endif
