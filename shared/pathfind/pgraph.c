#include "posix.h"

#ifndef CRTS_SERVER
#define CRTS_SERVER
#endif

#include <assert.h>
#include <limits.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "shared/pathfind/pathfind.h"
#include "shared/pathfind/pg_node.h"
#include "shared/pathfind/pgraph.h"
#include "shared/sim/chunk.h"
#include "shared/sim/tiles.h"
#include "shared/types/bheap.h"
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
	pg->heap = darr_init(sizeof(struct pgraph_heap_e));

	pg->chunk_date = pg->chunks->chunk_date;

	pg->unset = true;
}

struct pgraph *
pgraph_create(struct chunks *cnks, const struct point *goal, uint8_t et)
{
	struct pgraph *pg = calloc(1, sizeof(struct pgraph));

	pgraph_init(pg, cnks);
	pg->trav = et;
	pgraph_add_goal(pg, goal);

	return pg;
}

void
pgraph_add_goal(struct pgraph *pg, const struct point *g)
{
	struct pg_node *n;

	if (is_traversable(pg->chunks, g, pg->trav)) {
		n = pgn_summon(pg, g, 0);
		n->path_dist = 0;
		n->h_dist = 0;
		n->info |= ni_traversable;

		struct pgraph_heap_e e = { .v = 0, .i = n->index };
		darr_push(pg->heap, &e);

		if (pg->unset) {
			pg->unset = false;
			pg->chunk_date = pg->chunks->chunk_date;
		}
	} else {
		L("failed to add goal");
	}
}

static
enum iteration_result
reset_node_trav(void *_pg, void *_node)
{
	struct pg_node *n = _node;
	struct pgraph *pg = _pg;

	n->info &= ~ni_visited;

	if (is_traversable(pg->chunks, &n->p, pg->trav)) {
		n->info |= ni_traversable;

		if (!n->path_dist) {
			/* A goal was found */
			struct pgraph_heap_e e = { .v = 0, .i = n->index };
			darr_push(pg->heap, &e);

			return ir_cont;
		}
	} else {
		n->info &= ~ni_traversable;
	}

	n->path_dist = UINT16_MAX;
	n->h_dist = UINT32_MAX;
	n->parent = 0;

	return ir_cont;
}

void
pgraph_reset_terrain(struct pgraph *pg)
{
	darr_clear(pg->heap);
	hdarr_for_each(pg->nodes, pg, reset_node_trav);

	pg->chunk_date = pg->chunks->chunk_date;
}

static
enum iteration_result
reset_node_goals(void *_, void *_node)
{
	struct pg_node *n = _node;

	n->info &= ~ni_visited;
	n->path_dist = UINT16_MAX;
	n->h_dist = UINT32_MAX;
	n->parent = 0;

	return ir_cont;
}

void
pgraph_reset_goals(struct pgraph *pg)
{
	darr_clear(pg->heap);
	hdarr_for_each(pg->nodes, pg, reset_node_goals);

	pg->unset = true;
}

struct reset_node_hdist_ctx {
	struct pgraph *pg;
	const struct point *tgt;
	uint16_t smallest_dist;
};

static enum iteration_result
reset_node_hdist(void *_ctx, void *_e)
{
	struct pgraph_heap_e *e = _e;
	struct reset_node_hdist_ctx *ctx = _ctx;
	struct pg_node *n;

	n = hdarr_get_by_i(ctx->pg->nodes, e->i);
	n->h_dist = n->path_dist + square_dist(&n->p, ctx->tgt);

	return ir_cont;
}

void
pgraph_reset_hdist(struct pgraph *pg, const struct point *tgt)
{
	struct reset_node_hdist_ctx ctx = { pg, tgt, UINT16_MAX };

	darr_for_each(pg->heap, &ctx, reset_node_hdist);
}

void
pgraph_reset_all(struct pgraph *pg)
{
	hdarr_clear(pg->nodes);
	darr_clear(pg->heap);
	pg->unset = true;
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
