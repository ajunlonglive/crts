#include <limits.h>
#include <string.h>

#include "server/sim/pathfind/heap.h"
#include "server/sim/pathfind/mapping.h"
#include "server/sim/pathfind/pgraph.h"
#include "shared/sim/chunk.h"
#include "shared/types/hash.h"
#include "shared/util/log.h"
#include "shared/util/mem.h"

#define PGRAPH_HASH_CAP 4096
#define PGRAPH_HASH_BD  8


struct node *
pgraph_lookup(const struct path_graph *g, const struct point *p)
{
	const struct hash_elem *he;

	if ((he = hash_get(g->hash, p)) != NULL && he->init & HASH_VALUE_SET) {
		return g->nodes.e + he->val;
	} else {
		return NULL;
	}
}

int
find_or_create_node(struct path_graph *pg, const struct point *p)
{
	struct node *n;
	int off;

	union {
		void **vp;
		struct node **np;
	} nodes = { .np = &pg->nodes.e };

	if ((n = pgraph_lookup(pg, p)) == NULL) {
		off = get_mem(nodes.vp, sizeof(struct node), &pg->nodes.len, &pg->nodes.cap);
		n = pg->nodes.e + off;

		memset(n, 0, sizeof(struct node));
		n->p = *p;
		n->path_dist = INT_MAX;
		n->h_dist = INT_MAX;
		n->visited = 0;
		n->flow_calcd = 0;
		n->adj_calcd = 0;
		n->trav = pg->trav_getter(pg, n);

		hash_set(pg->hash, p, off);
	}

	return n - pg->nodes.e;
}

void
pgraph_create(struct path_graph *pg,
	struct chunks *cnks,
	const struct point *goal,
	int (*trav_getter)(struct path_graph *g, struct node *n),
	int res)
{
	struct node *n;
	int i;

	memset(pg, 0, sizeof(struct path_graph));

	pg->chunks = cnks;
	pg->trav_getter = trav_getter;
	pg->res = res;

	pg->hash = hash_init(PGRAPH_HASH_CAP, PGRAPH_HASH_BD, sizeof(struct point));

	heap_init(pg);

	if (goal != NULL) {
		pg->goal = *goal;

		i = find_or_create_node(pg, goal);
		n = pg->nodes.e + i;
		n->path_dist = 0;
		heap_push(pg, n);

		pg->possible = n->trav != trav_no;

		get_adjacent(pg, n);
		for (i = 0; i < 4; i++) {
			if (n->adj[i] == NULL_NODE) {
				continue;
			}
			n = pg->nodes.e + n->adj[i];

			n->path_dist = 1;
			heap_push(pg, n);
		}
	}
}
