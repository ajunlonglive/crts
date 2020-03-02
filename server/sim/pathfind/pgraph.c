#ifndef CRTS_SERVER
#define CRTS_SERVER
#endif

#define _XOPEN_SOURCE 500

#include <assert.h>
#include <limits.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "server/sim/pathfind/heap.h"
#include "server/sim/pathfind/pg_node.h"
#include "server/sim/pathfind/pgraph.h"
#include "shared/sim/chunk.h"
#include "shared/types/hash.h"
#include "shared/util/log.h"

#define PGRAPH_HASH_CAP 4096

struct pgraph *
pgraph_create(struct chunks *cnks, const struct point *goal)
{
	struct pgraph *pg = calloc(1, sizeof(struct pgraph));
	struct pg_node *n;

	pg->chunks = cnks;

	pg->nodes = hdarr_init(PGRAPH_HASH_CAP, sizeof(struct point), sizeof(struct pg_node), NULL);
	pg->possible = 1;

	heap_init(pg);

	if (goal != NULL) {
		pg->goal = *goal;
		n = pgn_summon(pg, &pg->goal, 0);
		n->path_dist = 0;
		heap_push(pg, n);
	}

	return pg;
}

void
pgraph_destroy(struct pgraph *pg)
{
	if (pg == NULL) {
		return;
	}

	hdarr_destroy(pg->nodes);
	darr_destroy(pg->heap);
	free(pg);
}
