#include <limits.h>
#include <string.h>

#include "util/log.h"
#include "mapping.h"
#include "heap.h"
#include "pgraph.h"
#include "types/hash.h"
#include "util/mem.h"
#include "sim/chunk.h"

struct node *pgraph_lookup(const struct path_graph *g, const struct point *p)
{
	const int *i;

	if ((i = hash_get(g->hash.h, p)) != NULL)
		return g->nodes.e + *i;
	else
		return NULL;
}

int find_or_create_node(struct path_graph *pg, const struct point *p)
{
	struct node *n;
	int *i, ii;

	union {
		void **vp;
		struct node **np;
	} nodes = { .np = &pg->nodes.e };

	union {
		void **vp;
		int **ip;
	} ints = { .ip = &pg->hash.e };

	if ((n = pgraph_lookup(pg, p)) == NULL) {
		ii = get_mem(nodes.vp, sizeof(struct node), &pg->nodes.len, &pg->nodes.cap);
		n = ii + pg->nodes.e;
		memset(n, 0, sizeof(struct node));

		n->p = *p;
		n->path_dist = INT_MAX;
		n->h_dist = INT_MAX;
		n->visited = 0;
		n->flow_calcd = 0;
		n->adj_calcd = 0;
		n->trav = pg->trav_getter(pg, n);

		ii = get_mem(ints.vp, sizeof(int), &pg->hash.len, &pg->hash.cap);
		i = ii + pg->hash.e;
		*i = n - pg->nodes.e;
		hash_set(pg->hash.h, p, i);
	}

	return n - pg->nodes.e;
}

void pgraph_create(struct path_graph *pg,
		   struct hash *cnks,
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

	pg->hash.h = hash_init(sizeof(struct point));
	pg->hash.cap = pg->hash.h->cap;
	pg->hash.e = calloc(pg->hash.cap, sizeof(int));

	heap_init(pg);

	if (goal != NULL) {
		pg->goal = *goal;

		i = find_or_create_node(pg, goal);
		n = pg->nodes.e + i;
		L("n: %p, %p, %d", n, pg->nodes.e, i);
		n->path_dist = 0;
		heap_push(pg, n);

		pg->possible = n->trav != trav_no;

		get_adjacent(pg, n);
		for (i = 0; i < 4; i++) {
			if (n->adj[i] == NULL_NODE)
				continue;
			n = pg->nodes.e + n->adj[i];

			n->path_dist = 1;
			heap_push(pg, n);
		}
	}
}
