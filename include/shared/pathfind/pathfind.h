#ifndef SHARED_PATHFIND_PATHFIND_H
#define SHARED_PATHFIND_PATHFIND_H

#include "shared/pathfind/pgraph.h"
#include "shared/sim/chunk.h"
#include "shared/sim/ent.h"
#include "shared/types/geom.h"
#include "shared/types/result.h"

struct pgraph_heap_e {
	uint32_t v;
	size_t i;
};

typedef enum result ((*astar_callback)(void *ctx, const struct point *pos));

#define ASTAR_DEF_RADIUS 128

enum result astar(struct pgraph *pg, const struct point *e, void *ctx,
	astar_callback callback, uint32_t radius);
enum result pathfind(struct pgraph *pg, struct point *p);
enum result ent_pathfind(struct ent *e);
#endif
