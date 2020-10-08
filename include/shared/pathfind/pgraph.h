#ifndef SHARED_PATHFIND_PGRAPH_H
#define SHARED_PATHFIND_PGRAPH_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "shared/types/geom.h"
#include "shared/types/darr.h"
#include "shared/types/hdarr.h"

struct pgraph {
	struct darr *heap;
	struct hdarr *nodes;
	struct chunks *chunks;
	size_t chunk_date;
	uint8_t trav;
	bool unset;
};

struct pgraph *pgraph_create(struct chunks *cnks, const struct point *goal, uint8_t et);
void pgraph_init(struct pgraph *pg, struct chunks *cnks);
void pgraph_destroy(struct pgraph *pg);
void pgraph_add_goal(struct pgraph *pg, const struct point *g);
void pgraph_reset_hdist(struct pgraph *pg, const struct point *tgt);
void pgraph_reset_terrain(struct pgraph *pg);
void pgraph_reset_goals(struct pgraph *pg);
void pgraph_reset_all(struct pgraph *pg);
#endif
