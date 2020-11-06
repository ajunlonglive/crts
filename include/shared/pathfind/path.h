#ifndef SHARED_PATHFIND_ABSTRACT_PATH_H
#define SHARED_PATHFIND_ABSTRACT_PATH_H

#include "shared/pathfind/abstract_graph.h"

struct ag_path {
	struct point comp[MAXPATH_ABSTRACT];
	uint8_t node[MAXPATH_ABSTRACT];
};

enum pathfind_path_flags {
	ppf_local_done    = 1 << 0,
	ppf_abstract_done = 1 << 1,
	ppf_initialized   = 1 << 2,
};

struct pathfind_path {
	struct ag_path abstract;
	uint8_t local[MAXPATH_LOCAL];
	struct point goal;
	uint8_t local_i, local_len, flags;
	uint16_t abstract_i, abstract_len;
};
#endif
