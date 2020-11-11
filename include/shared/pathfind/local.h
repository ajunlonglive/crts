#ifndef SHARED_PATHFIND_LOCAL_H
#define SHARED_PATHFIND_LOCAL_H

#include "shared/pathfind/abstract.h"

bool astar_local(const struct ag_component *agc, uint8_t s, uint8_t goal,
	uint8_t path[MAXPATH_LOCAL], uint8_t *pathlen);
bool astar_local_possible(const struct ag_component *agc, uint8_t s, uint8_t goal);
#endif
