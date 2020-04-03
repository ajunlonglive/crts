#ifndef CRTS_SERVER
#define CRTS_SERVER
#endif

#define _XOPEN_SOURCE 500

#include "server/sim/pathfind/heap.h"
#include "server/sim/pathfind/pathfind.h"
#include "server/sim/pathfind/pg_node.h"
#include "shared/math/geom.h"
#include "shared/types/darr.h"
#include "shared/types/hdarr.h"
#include "shared/types/result.h"
#include "shared/util/log.h"
#include "shared/util/mem.h"

#define MAXNODES 2 << 12

enum result
astar(struct pgraph *pg, const struct point *e, void *ctx, astar_callback callback)
{
	struct pg_node *n, *c;
	enum result r;
	size_t ni, *ip, i;
	uint32_t tdist;

	while (1) {
		heap_sort(pg);

		if (darr_len(pg->heap) <= 0) {
			return rs_fail;
		} else if (heap_peek(pg)->info & ni_visited) {
			heap_pop(pg);
			continue;
		}

		ip = darr_get(pg->heap, pg->smallest);
		ni = *ip;
		n = hdarr_get_by_i(pg->nodes, ni);
		n->info |= ni_visited;

		if (callback && (r = callback(ctx, &n->p)) != rs_cont) {
			return r;
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
				c->h_dist = tdist + (e ? square_dist(&c->p, e) : 0);
				c->parent = ni;
			}

			if (!(c->info & ni_visited)) {
				heap_push(pg, c);
			}
		}

		if (hdarr_len(pg->nodes) > MAXNODES) {
			return rs_fail;
		} else if (e && points_equal(&n->p, e)) {
			return rs_done;
		}
	}
}

static enum result
pgraph_next_point(struct pgraph *pg, struct point *p)
{
	struct pg_node *n, *pn;
	enum result pr;

	if ((n = hdarr_get(pg->nodes, p)) == NULL) {
		if ((pr = astar(pg, p, NULL, NULL)) != rs_done) {
			return pr;
		}

		n = hdarr_get(pg->nodes, p);
	}

	if ((pn = hdarr_get_by_i(pg->nodes, n->parent)) == NULL) {
		return rs_done;
	} else {
		*p = pn->p;
		return rs_cont;
	}
}

enum result
pathfind(struct pgraph *pg, struct point *p)
{
	if (pg->chunks->chunk_date != pg->chunk_date) {
		pg->chunk_date = pg->chunks->chunk_date;
		pgraph_reset(pg);
		//L("reset pgraph %p", pg);
	}

	if (!pg->possible) {
		return rs_fail;
	} else if (points_equal(&pg->goal, p)) {
		return rs_done;
	} else {
		return pgraph_next_point(pg, p);
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
