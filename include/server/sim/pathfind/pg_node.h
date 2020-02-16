#ifndef _PATHFIND_PG_NODE_H
#define _PATHFIND_PG_NODE_H

#include <stdbool.h>
#include <stdint.h>

#include "server/sim/pathfind/pgraph.h"
#include "shared/types/geom.h"

#define NULL_NODE -1

struct pg_node {
	bool traversable;
	uint8_t visited;
	uint8_t adj_calcd;
	int16_t adj[4];
	uint16_t path_dist;
	uint32_t h_dist;
	const struct pg_node *parent;
	struct point p;
};

void pgn_summon_adj(struct pgraph *pg, struct pg_node *n);
uint16_t pgn_summon(struct pgraph *pg, const struct point *p, const struct pg_node *parent);
#endif
