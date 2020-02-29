#ifndef __PATHFIND_H
#define __PATHFIND_H

#include "server/sim/pathfind/pgraph.h"
#include "shared/sim/chunk.h"
#include "shared/types/geom.h"

enum result pathfind(struct pgraph *pg, struct point *p);
#endif
