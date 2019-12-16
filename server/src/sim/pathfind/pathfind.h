#ifndef __PATHFIND_H
#define __PATHFIND_H
#include "types/geom.h"
#include "pgraph.h"

struct path_graph *tile_pg_create(struct hash *cnks, const struct point *goal);
struct path_graph *chunk_pg_create(struct hash *cnks, const struct point *goal);
int pathfind(struct path_graph *pg, struct point *p);
#endif
