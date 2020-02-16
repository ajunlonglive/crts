#ifndef __PATHFIND_H
#define __PATHFIND_H

#include "server/sim/pathfind/pgraph.h"
#include "shared/sim/chunk.h"
#include "shared/types/geom.h"

enum pathfind_result {
	pr_cont,
	pr_done,
	pr_fail
};

enum pathfind_result pathfind(struct pgraph *pg, struct point *p);
#endif
