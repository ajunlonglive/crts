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
#define PGRAPH_HASH_BD  8

struct pg_node *
pgraph_lookup(const struct pgraph *g, const struct point *p)
{
	const size_t *val;

	if ((val = hash_get(g->hash, p)) != NULL) {
		return g->nodes.e + *val;
	} else {
		return NULL;
	}
}

struct pgraph *
pgraph_create(struct chunks *cnks, const struct point *goal)
{
	size_t off;
	struct pgraph *pg = calloc(1, sizeof(struct pgraph));
	struct pg_node *n;

	pg->chunks = cnks;

	pg->hash = hash_init(PGRAPH_HASH_CAP, PGRAPH_HASH_BD, sizeof(struct point));
	pg->possible = 1;

	heap_init(pg);

	if (goal != NULL) {
		pg->goal = *goal;
		off = pgn_summon(pg, &pg->goal, NULL);
		n = pg->nodes.e + off;
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

	hash_destroy(pg->hash);
	free(pg->heap.e);
	free(pg->nodes.e);
	free(pg);
}
