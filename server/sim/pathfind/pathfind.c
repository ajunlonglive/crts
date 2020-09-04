#include "posix.h"

#ifndef CRTS_SERVER
#define CRTS_SERVER
#endif

#include "server/sim/pathfind/heap.h"
#include "server/sim/pathfind/pathfind.h"
#include "server/sim/pathfind/pg_node.h"
#include "shared/constants/globals.h"
#include "shared/math/geom.h"
#include "shared/types/darr.h"
#include "shared/types/hdarr.h"
#include "shared/types/result.h"
#include "shared/util/log.h"
#include "shared/util/mem.h"

enum result
astar(struct pgraph *pg, const struct point *e, void *ctx,
	astar_callback callback, uint32_t radius)
{
	struct pg_node *n, *c;
	enum result r;
	size_t ni, *ip, i;
	uint32_t tdist;

	if (darr_len(pg->heap) <= 0) {
		/* TODO: investigate how this occurs */
		L("attempting to pathfind with no goal");
		return rs_fail;
	}

	heap_sort(pg);
	struct circle extent = { .center = heap_peek(pg)->p, .r = radius };

	while (1) {
		heap_sort(pg);

		if (darr_len(pg->heap) <= 0) {
			/* L("pathfind failing: no more valid moves"); */
			return rs_fail;
		} else if (heap_peek(pg)->info & ni_visited) {
			heap_pop(pg);
			continue;
		}

		ip = darr_get(pg->heap, pg->smallest);
		ni = *ip;
		n = hdarr_get_by_i(pg->nodes, ni);
		n->info |= ni_visited;

		if (!point_in_circle(&n->p, &extent)) {
			/* L("out of range, %d, %d", extent.center.x, extent.center.y); */
			continue;
		}

		pgn_summon_adj(pg, n);
		n = hdarr_get_by_i(pg->nodes, ni);

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
				heap_push(pg, c);
			}
		}

		if (callback && (r = callback(ctx, &n->p)) != rs_cont) {
			return r;
		} else if (e && points_equal(&n->p, e)) {
			return rs_done;
		}
	}

	return rs_cont;
}

enum result
pathfind(struct pgraph *pg, struct point *p)
{
	struct pg_node *n;
	enum result pr;

	if (pg->chunks->chunk_date != pg->chunk_date) {
		pg->chunk_date = pg->chunks->chunk_date;
		pgraph_reset_terrain(pg);
	}

	if (!(n = hdarr_get(pg->nodes, p)) || !(n->info & ni_visited)) {
		pgraph_reset_hdist(pg, p);

		if ((pr = astar(pg, p, NULL, NULL, ASTAR_DEF_RADIUS)) != rs_done) {
			return pr;
		}

		n = hdarr_get(pg->nodes, p);
	}

	if (!n->path_dist) {
		return rs_done;
	} else {
		*p = ((struct pg_node *)hdarr_get_by_i(pg->nodes, n->parent))->p;
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
