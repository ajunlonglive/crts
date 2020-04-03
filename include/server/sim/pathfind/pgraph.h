#ifndef __PATHFIND_PGRAPH_H
#define __PATHFIND_PGRAPH_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "shared/types/geom.h"
#include "shared/types/darr.h"
#include "shared/types/hdarr.h"

struct pgraph {
	struct point goal;
	struct darr *heap;
	struct hdarr *nodes;
	struct chunks *chunks;
	size_t chunk_date;
	size_t smallest;
	uint8_t trav;
	bool possible;
	bool unset;
};

struct pgraph *pgraph_create(struct chunks *cnks, const struct point *goal, uint8_t et);
void pgraph_init(struct pgraph *pg, struct chunks *cnks);
void pgraph_set(struct pgraph *pg, const struct point *g, uint8_t trav);
void pgraph_reset(struct pgraph *pg);
void pgraph_destroy(struct pgraph *pg);
void pgraph_add_goal(struct pgraph *pg, const struct point *g);
#endif
