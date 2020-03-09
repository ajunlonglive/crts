#ifndef __PATHFIND_PGRAPH_H
#define __PATHFIND_PGRAPH_H

#include <stdbool.h>
#include <stddef.h>

#include "shared/types/geom.h"
#include "shared/types/darr.h"
#include "shared/types/hdarr.h"

struct pgraph {
	struct darr *heap;

	struct hdarr *nodes;
	struct chunks *chunks;

	struct point goal;

	bool possible;
	size_t chunk_date;
	size_t smallest;
};

struct pgraph *pgraph_create(struct chunks *cnks, const struct point *goal);
void pgraph_reset(struct pgraph *pg);
void pgraph_destroy(struct pgraph *pg);
#endif
