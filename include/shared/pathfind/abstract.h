#ifndef SHARED_PATHFIND_ABSTRACT_H
#define SHARED_PATHFIND_ABSTRACT_H

#include <stdbool.h>
#include <stdint.h>

#include "shared/pathfind/abstract_graph.h"
#include "shared/pathfind/path.h"

#define MAX_INTER_EDGES 32

/* TODO: remove this circular dep */
#define CHUNK_SIZE 16
#define CHUNK_PERIM (CHUNK_SIZE * 4)
struct chunks;
struct chunk;

enum ag_node_flags {
	agn_filled  = 1 << 0,
	agn_visited = 1 << 1,
};

/* for temporary insertion of start / goal nodes */
enum ag_node_special_indices {
	tmp_node = CHUNK_PERIM,
};

struct ag_node {
	uint8_t weights[MAX_INTER_EDGES];
	uint8_t adjacent[MAX_INTER_EDGES];
	/* struct pg_abstract_edge inter_edges[MAX_INTER_EDGES]; */
	uint8_t edges, flags;
};

struct ag_component {
	struct ag_node nodes[CHUNK_PERIM + 1];
	uint8_t edges[(CHUNK_SIZE * CHUNK_SIZE) / 2];
	struct point pos;
};

struct ag_tmp_component_key {
	struct point pos;
	uint8_t node;
};

struct ag_key {
	uint16_t component;
	uint16_t node;
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
