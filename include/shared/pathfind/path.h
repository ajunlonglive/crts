#ifndef SHARED_PATHFIND_ABSTRACT_PATH_H
#define SHARED_PATHFIND_ABSTRACT_PATH_H

#include "shared/pathfind/abstract_graph.h"

struct ag_path {
	struct point comp[MAXPATH_ABSTRACT];
	uint8_t node[MAXPATH_ABSTRACT];
	uint16_t len, i;
};

enum pathfind_path_flags {
	ppf_local_done    = 1 << 0,
	ppf_abstract_done = 1 << 1,
	ppf_initialized   = 1 << 2,
	ppf_dirty         = 1 << 3,
};

struct pathfind_path {
	struct ag_path abstract;
	struct {
		uint8_t idx[MAXPATH_LOCAL];
		uint8_t len, i;
	} local;
	struct point goal;
	uint8_t flags;
};
#endif
