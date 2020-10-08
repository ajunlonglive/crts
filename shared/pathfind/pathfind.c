#include "posix.h"

#ifndef CRTS_SERVER
#define CRTS_SERVER
#endif

#include "shared/constants/globals.h"
#include "shared/math/geom.h"
#include "shared/pathfind/pathfind.h"
#include "shared/pathfind/pg_node.h"
#include "shared/types/bheap.h"
#include "shared/types/darr.h"
#include "shared/types/hdarr.h"
#include "shared/types/result.h"
#include "shared/util/log.h"
#include "shared/util/mem.h"
#include "tracy.h"

enum result
astar(struct pgraph *pg, const struct point *e, void *ctx,
	astar_callback callback, uint32_t radius)
{
	TracyCZoneAutoS;
	struct pg_node *n, *c;
	enum result r;
	size_t i, ni;
	uint32_t tdist;

	if (darr_len(pg->heap) <= 0) {
		/* TODO: investigate how this occurs */
		L("attempting to pathfind with no goal");
		TracyCZoneAutoE;
		return rs_fail;
	}

	struct circle extent = {
		.center = ((struct pg_node *)hdarr_get_by_i(pg->nodes,
			((struct pgraph_heap_e *)bheap_peek(pg->heap))->i))->p,
		.r = radius
	};

	while (1) {
		if (darr_len(pg->heap) <= 0) {
			/* L("pathfind failing: no more valid moves"); */
			TracyCZoneAutoE;
			return rs_fail;
		}

		ni = ((struct pgraph_heap_e *)bheap_peek(pg->heap))->i;
		n = hdarr_get_by_i(pg->nodes, ni);
		bheap_pop(pg->heap);

		if (n->info & ni_visited) {
			continue;
		}

		n->info |= ni_visited;

		if (!point_in_circle(&n->p, &extent)) {
			/* L("out of range, %d, %d", extent.center.x, extent.center.y); */
			continue;
		}

		pgn_summon_adj(pg, n);
		n = hdarr_get_by_i(pg->nodes, ni);

		TracyCZoneN(tctx_neighbour_loop, "neighbour loop", true);
		for (i = 0; i < 4; i++) {
			c = hdarr_get_by_i(pg->nodes, n->adj[i]);

			if (!(c->info & ni_traversable)) {
				continue;
			}

			if ((tdist = n->path_dist + 1) < c->path_dist) {
				c->path_dist = tdist;
				tdist += e ? square_dist(&c->p, e) : 0;
				c->h_dist = tdist;
				c->parent = ni;
			}

			if (!(c->info & ni_visited)) {
				struct pgraph_heap_e e = { .v = c->h_dist, .i = c->index };
				bheap_push(pg->heap, &e);
			}
		}
		TracyCZoneEnd(tctx_neighbour_loop);

		if (callback && (r = callback(ctx, &n->p)) != rs_cont) {
			TracyCZoneAutoE;
			return r;
		} else if (e && points_equal(&n->p, e)) {
			TracyCZoneAutoE;
			return rs_done;
		}
	}

	TracyCZoneAutoE;
	return rs_cont;
}

enum result
pathfind(struct pgraph *pg, struct point *p)
{
	TracyCZoneAutoS;
	struct pg_node *n;
	enum result pr;

	if (pg->chunks->chunk_date != pg->chunk_date) {
		pg->chunk_date = pg->chunks->chunk_date;
		pgraph_reset_terrain(pg);
	}

	if (!(n = hdarr_get(pg->nodes, p)) || !(n->info & ni_visited)) {
		pgraph_reset_hdist(pg, p);

		if ((pr = astar(pg, p, NULL, NULL, ASTAR_DEF_RADIUS)) != rs_done) {
			TracyCZoneAutoE;
			return pr;
		}

		n = hdarr_get(pg->nodes, p);
	}

	if (!n->path_dist) {
		TracyCZoneAutoE;
		return rs_done;
	} else {
		*p = ((struct pg_node *)hdarr_get_by_i(pg->nodes, n->parent))->p;
		TracyCZoneAutoE;
		return rs_cont;
	}
}

enum result
ent_pathfind(struct ent *e)
{
	enum result r;

	switch (r = pathfind(e->pg, &e->pos)) {
	case rs_cont:
		e->state |= es_modified;
		break;
	case rs_fail:
	case rs_done:
		break;
	}

	return r;
}
