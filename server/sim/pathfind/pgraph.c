#include <assert.h>
#include <limits.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>

#include "server/sim/pathfind/heap.h"
#include "server/sim/pathfind/pg_node.h"
#include "server/sim/pathfind/pgraph.h"
#include "shared/sim/chunk.h"
#include "shared/sim/chunk.h"
#include "shared/types/hash.h"
#include "shared/util/log.h"

#define PGRAPH_HASH_CAP 4096
#define PGRAPH_HASH_BD  8

struct pg_node *
pgraph_lookup(const struct pgraph *g, const struct point *p)
{
	const struct hash_elem *he;

	if ((he = hash_get(g->hash, p)) != NULL && he->init & HASH_VALUE_SET) {
		return g->nodes.e + he->val;
	} else {
		return NULL;
	}
}

struct pgraph *
pgraph_create(struct chunks *cnks, const struct point *goal)
{
	struct pgraph *pg = calloc(1, sizeof(struct pgraph));

	pg->chunks = cnks;

	pg->hash = hash_init(PGRAPH_HASH_CAP, PGRAPH_HASH_BD, sizeof(struct point));

	heap_init(pg);

	if (goal != NULL) {
		pg->goal = *goal;
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
