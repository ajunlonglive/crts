#include "mapping.h"
#include "heap.h"
#include "sim/chunk.h"

void get_adjacent(struct path_graph *pg, struct node *n)
{
	if (n->adj_calcd)
		return;

	//   3
	// 1 n 0
	//   2

	const struct point *s = &n->p, ps[] = {
		{ s->x + pg->res, s->y           },
		{ s->x - pg->res, s->y           },
		{ s->x,           s->y + pg->res },
		{ s->x,           s->y - pg->res },
	};
	int anodes[] = {
		find_or_create_node(pg, &ps[0]),
		find_or_create_node(pg, &ps[1]),
		find_or_create_node(pg, &ps[2]),
		find_or_create_node(pg, &ps[3]),
	}, t[] = {
		(pg->nodes.e + anodes[0])->trav,
		(pg->nodes.e + anodes[0])->trav,
		(pg->nodes.e + anodes[0])->trav,
		(pg->nodes.e + anodes[0])->trav,
		n->trav
	};

#define ct(i, bm) (i & bm) == bm
	n->adj[0] = (ct(t[4], trav_ne) || ct(t[4], trav_se) || ct(t[4], trav_ew)) &&
		    (ct(t[0], trav_nw) || ct(t[0], trav_sw) || ct(t[0], trav_ew))
		    ? anodes[0] : NULL_NODE;
	n->adj[1] = (ct(t[4], trav_nw) || ct(t[4], trav_sw) || ct(t[4], trav_ew)) &&
		    (ct(t[1], trav_ne) || ct(t[1], trav_se) || ct(t[1], trav_ew))
		    ? anodes[1] : NULL_NODE;
	n->adj[2] = (ct(t[4], trav_se) || ct(t[4], trav_sw) || ct(t[4], trav_ns)) &&
		    (ct(t[2], trav_ne) || ct(t[2], trav_nw) || ct(t[2], trav_ns))
		    ? anodes[2] : NULL_NODE;
	n->adj[3] = (ct(t[4], trav_ne) || ct(t[4], trav_nw) || ct(t[4], trav_ns)) &&
		    (ct(t[3], trav_se) || ct(t[3], trav_sw) || ct(t[3], trav_ns))
		    ? anodes[3] : NULL_NODE;
#undef ct
}

int brushfire(struct path_graph *pg, const struct point *e)
{
	struct node *n, *c;
	int i, tdist;

	if (heap_peek(pg)->visited)
		return heap_pop(pg);

	get_adjacent(pg, pg->nodes.e + pg->heap.e[0]);
	n = pg->nodes.e + heap_pop(pg);

	n->visited = 1;

	for (i = 0; i < 4; i++) {
		if (n->adj[i] == NULL_NODE)
			continue;

		c = pg->nodes.e + n->adj[i];

		if ((tdist = n->path_dist + 1) < c->path_dist) {
			c->path_dist = tdist;
			c->h_dist = tdist + square_dist(&c->p, e);
		}

		if (!c->visited)
			heap_push(pg, c);
	}

	return (n->p.x == e->x && n->p.y == e->y) || pg->heap.len <= 0 ? 1 : 0;
}

void reset_graph_hdist(struct path_graph *pg, const struct point *p)
{
	size_t i;
	struct node *n;

	for (i = 0; i < pg->nodes.len; i++) {
		n = &pg->nodes.e[i];

		n->h_dist = n->path_dist + square_dist(&n->p, p);
	}

	heap_sort(pg);
}
