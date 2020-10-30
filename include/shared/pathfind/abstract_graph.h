#ifndef SHARED_PATHFIND_ABSTRACT_GRAPH_H
#define SHARED_PATHFIND_ABSTRACT_GRAPH_H

#include "shared/pathfind/trav.h"
#include "shared/types/geom.h"

#define MAXPATH 64

struct abstract_graph {
	struct hdarr *components;
	struct hash *visited;
	struct darr *heap;
	enum trav_type trav;
};

struct ag_path {
	struct point comp[MAXPATH];
	uint8_t node[MAXPATH];
};

#endif
