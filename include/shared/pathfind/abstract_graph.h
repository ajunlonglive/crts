#ifndef SHARED_PATHFIND_ABSTRACT_GRAPH_H
#define SHARED_PATHFIND_ABSTRACT_GRAPH_H

#include "shared/pathfind/trav.h"
#include "shared/types/geom.h"

#define MAXPATH_ABSTRACT 1024
#define MAXPATH_LOCAL 64

struct abstract_graph {
	struct hdarr *components;
	struct hash *visited;
	struct darr *heap;
	struct hdarr *tmp_components;
	struct darr *paths;
	struct darr *free_paths;
	enum trav_type trav;
};
#endif
