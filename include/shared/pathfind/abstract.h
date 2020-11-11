#ifndef SHARED_PATHFIND_ABSTRACT_H
#define SHARED_PATHFIND_ABSTRACT_H

#include <stdbool.h>
#include <stdint.h>

#include "shared/pathfind/abstract_graph.h"
#include "shared/pathfind/path.h"

#define MAX_REGIONS 32
#define NULL_REGION 0
#define MAX_REGION_EDGES 8
#define CHUNK_PERIM (16 * 4)

/* edge -> 1 byte
 *
 * 2 bits -> side   0-3
 * 4 bits -> region 0-31
 * 2 bits -> unused
 */

struct ag_region {
	uint8_t edges[MAX_REGION_EDGES];
	uint8_t entrances[MAX_REGION_EDGES];
	uint8_t center;
	uint8_t edges_len;
};

struct ag_component {
	/* (256 positions * 4 bits per position (RLUD)) / 8 bits == 128 bytes */
	uint8_t adj_map[128];
	/* (256 positions * 4 bits per position (0-31)) / 8 bits == 128 bytes */
	uint8_t region_map[128];
	struct ag_region regions[MAX_REGIONS];
	struct point pos;
	uint8_t regions_len;
};

struct ag_key {
	uint16_t component;
	uint8_t region;
	uint8_t edge_i;
};

union ag_val {
	uint64_t i;
	struct {
		struct ag_key prev;
		uint16_t d;
		uint16_t _;
	} s;
};

struct ag_heap_e {
	uint32_t d;
	struct ag_key key;
};

bool astar_abstract(struct abstract_graph *ag, const struct point *s,
	const struct point *g, struct ag_path *path, uint16_t *pathlen);
#endif
