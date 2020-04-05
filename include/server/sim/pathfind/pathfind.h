#ifndef SERVER_SIM_PATHFIND_PATHFIND_H
#define SERVER_SIM_PATHFIND_PATHFIND_H

#include "server/sim/pathfind/pgraph.h"
#include "shared/sim/chunk.h"
#include "shared/sim/ent.h"
#include "shared/types/geom.h"
#include "shared/types/result.h"

#define PATHFIND_MAXNODES 1 << 12

typedef enum result ((*astar_callback)(void *ctx, const struct point *pos));

enum result astar(struct pgraph *pg, const struct point *e, void *ctx,
	astar_callback callback);
enum result pathfind(struct pgraph *pg, struct point *p);
enum result ent_pathfind(struct ent *e);
#endif
