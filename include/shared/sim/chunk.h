#ifndef SHARED_SIM_CHUNK_H
#define SHARED_SIM_CHUNK_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "shared/math/geom.h"
#include "shared/pathfind/abstract_graph.h"
#include "shared/types/darr.h"
#include "shared/types/hash.h"
#include "shared/types/hdarr.h"

enum tile {
	tile_sea,
	tile_coast,
	tile_plain,
	tile_rock,
	tile_dirt,
	tile_tree,
	tile_old_tree,
	tile_fire,
	tile_ash,

	tile_count,
};

#define CHUNK_SIZE 16

_Static_assert(tile_count < 256, "increase type of tiles in chunk");

struct chunk {
	uint8_t tiles[CHUNK_SIZE][CHUNK_SIZE];
	float heights[CHUNK_SIZE][CHUNK_SIZE];
	uint8_t energy[CHUNK_SIZE][CHUNK_SIZE];
	uint16_t ent_height[CHUNK_SIZE][CHUNK_SIZE];
	size_t last_touched;
	bool touched_this_tick;
	struct point pos;
};

struct chunks {
	struct abstract_graph ag;
	struct hdarr hd;
	struct hash functional_tiles;
	struct hash functional_tiles_buf;
	size_t chunk_date;
};

/* TODO: replace hash value type with uint64_t so we always know how many bits
 * it has
 */
_Static_assert(sizeof(size_t) == 8, "wrong size size_t");

union functional_tile {
	size_t val;

	struct {
		uint16_t type;
		uint16_t motivator;
		uint16_t age;
		uint16_t _pad;
	} ft;
};

void chunks_init(struct chunks *cnks);
struct point nearest_chunk(const struct point *p);
void chunks_destroy(struct chunks *cnks);
struct chunk *get_chunk(struct chunks *cnks, const struct point *p);
struct chunk *get_chunk_at(struct chunks *cnks, const struct point *p);
void set_chunk(struct chunks *cnks, struct chunk *ck);
void touch_chunk(struct chunks *cnks, struct chunk *ck);
#endif
