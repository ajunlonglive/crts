#define _XOPEN_SOURCE 500

#include "server/sim/pathfind/heap.h"
#include "server/sim/pathfind/pathfind.h"
#include "server/sim/pathfind/pg_node.h"
#include "shared/math/geom.h"
#include "shared/util/log.h"
#include "shared/util/mem.h"

#define GIVE_UP_AFTER 8192

static enum pathfind_result
brushfire(struct pgraph *pg, const struct point *e)
{
	struct pg_node *n, *c;
	int i, tdist, j = 0;
	enum pathfind_result r = pr_cont;

	while (!r) {
		if (++j > GIVE_UP_AFTER || pg->heap.len <= 0) {
			return pr_fail;
		}

		heap_sort(pg);

		if (heap_peek(pg)->visited) {
			heap_pop(pg);
			continue;
		}

		pgn_summon_adj(pg, pg->nodes.e + pg->heap.e[0]);
		n = pg->nodes.e + pg->heap.e[0];

		n->visited = 1;

		for (i = 0; i < 4; i++) {
			if (n->adj[i] == NULL_NODE) {
				continue;
			}

			c = pg->nodes.e + n->adj[i];

			if ((tdist = n->path_dist + 1) < c->path_dist) {
				c->path_dist = tdist;
				c->h_dist = tdist + square_dist(&c->p, e);
				c->parent = n - pg->nodes.e;
			}

			if (!c->visited) {
				heap_push(pg, c);
			}
		}

		if (points_equal(&n->p, e)) {
			r = pr_done;
		} else if (pg->heap.len <= 0) {
			r = pr_fail;
		}
	}

	return r;
}

static enum pathfind_result
pgraph_next_point(struct pgraph *pg, struct point *p)
{
	struct pg_node *n, *pn;

	if ((n = pgraph_lookup(pg, p)) == NULL) {
		if (brushfire(pg, p) > 1) {
			return pr_fail;
		}

		n = pgraph_lookup(pg, p);
	}

	if ((pn = pg->nodes.e + n->parent) == n) {
		return pr_done;
	} else {
		*p = pn->p;
		return pr_cont;
	}
}

enum pathfind_result
pathfind(struct pgraph *pg, struct point *p)
{
	if (!pg->possible) {
		return pr_fail;
	} else if (pg->goal.x == p->x && pg->goal.y == p->y) {
		return pr_done;
	} else {
		return pgraph_next_point(pg, p);
	}
}
