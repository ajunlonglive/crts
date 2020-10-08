#ifndef SHARED_PATHFIND_PG_NODE_H
#define SHARED_PATHFIND_PG_NODE_H

#include <stdbool.h>
#include <stdint.h>

#include "shared/pathfind/pgraph.h"
#include "shared/types/geom.h"

enum node_info {
	ni_visited     = 1 << 0,
	ni_traversable = 1 << 1,
	ni_adj_calcd   = 1 << 2,
};

struct pg_node {
	size_t adj[4];
	struct point p;
	size_t index;
	uint32_t h_dist;
	uint16_t path_dist;
	uint16_t parent;
	uint8_t info;
};

void pgn_summon_adj(struct pgraph *pg, struct pg_node *n);
struct pg_node *pgn_summon(struct pgraph *pg, const struct point *p, size_t parent_index);
#endif
