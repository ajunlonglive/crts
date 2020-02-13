#ifndef __PATHFIND_H
#define __PATHFIND_H

#include "server/sim/pathfind/pgraph.h"
#include "shared/sim/chunk.h"
#include "shared/types/geom.h"

struct path_graph *tile_pg_create(struct chunks *cnks, const struct point *goal);
struct path_graph *chunk_pg_create(struct chunks *cnks, const struct point *goal);
int pathfind(struct path_graph *pg, struct point *p);
#endif
