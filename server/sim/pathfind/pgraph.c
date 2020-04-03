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
#include "server/sim/terrain.h"
#include "server/sim/pathfind/pg_node.h"
#include "server/sim/pathfind/pgraph.h"
#include "shared/sim/chunk.h"
#include "shared/types/hash.h"
#include "shared/util/log.h"

#define PGRAPH_HASH_CAP 4096

void
pgraph_init(struct pgraph *pg, struct chunks *cnks)
{
	memset(pg, 0, sizeof(struct pgraph));

	pg->chunks = cnks;
	pg->nodes = hdarr_init(PGRAPH_HASH_CAP, sizeof(struct point),
		sizeof(struct pg_node), NULL);
	pg->heap = darr_init(sizeof(size_t));

	pg->unset = true;
}

struct pgraph *
pgraph_create(struct chunks *cnks, const struct point *goal, uint8_t et)
{
	struct pgraph *pg = calloc(1, sizeof(struct pgraph));

	pgraph_init(pg, cnks);
	pgraph_set(pg, goal, et);

	return pg;
}

void
pgraph_set(struct pgraph *pg, const struct point *g, uint8_t et)
{
	pg->trav = et;
	pg->goal = *g;
	pg->unset = false;

	pgraph_reset(pg);
}

void
pgraph_add_goal(struct pgraph *pg, const struct point *g)
{
	struct pg_node *n;

	if (is_traversable(pg->chunks, g, pg->trav)) {
		n = pgn_summon(pg, &pg->goal, 0);
		n->path_dist = 0;
		heap_push(pg, n);
	}
}

void
pgraph_reset(struct pgraph *pg)
{
	hdarr_clear(pg->nodes);
	darr_clear(pg->heap);

	pg->chunk_date = pg->chunks->chunk_date;
	pg->smallest = 0;

	pg->possible = is_traversable(pg->chunks, &pg->goal, pg->trav);

	pgraph_add_goal(pg, &pg->goal);
}

void
pgraph_destroy(struct pgraph *pg)
{
	if (pg == NULL) {
		return;
	}

	hdarr_destroy(pg->nodes);
	darr_destroy(pg->heap);
}
