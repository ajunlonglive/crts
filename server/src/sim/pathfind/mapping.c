#include "mapping.h"
#include "heap.h"
#include "sim/chunk.h"
#include "pgraph.h"
#include "util/log.h"

void get_adjacent(struct path_graph *pg, struct node *n)
{
	int noff = n - pg->nodes.e;

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
	};

	// reset n, since it might have been invalidated by realloc
	n = pg->nodes.e + noff;

	int t[] = {
		(pg->nodes.e + anodes[0])->trav,
		(pg->nodes.e + anodes[1])->trav,
		(pg->nodes.e + anodes[2])->trav,
		(pg->nodes.e + anodes[3])->trav,
		n->trav
	};


#define ct(i, bm) ((i & bm) == bm)
	n->adj[0] = ct(t[4], trav_e) && ct(t[0], trav_w) ? anodes[0] : NULL_NODE;
	n->adj[1] = ct(t[4], trav_w) && ct(t[1], trav_e) ? anodes[1] : NULL_NODE;
	n->adj[2] = ct(t[4], trav_s) && ct(t[2], trav_n) ? anodes[2] : NULL_NODE;
	n->adj[3] = ct(t[4], trav_n) && ct(t[3], trav_s) ? anodes[3] : NULL_NODE;
#undef ct

	n->adj_calcd = 1;
}

static float guide_penalty(struct path_graph *guide, const struct node *n)
{
	struct point pp = nearest_chunk(&n->p);
	struct node *p;

	if ((p = pgraph_lookup(guide, &pp)) == NULL)
		return 99999.0f;
	float fpd = (float)p->path_dist;

	if (p->path_dist <= 0)
		return 0.0f;
	else
		return fpd * fpd;
}

int brushfire(struct path_graph *pg, struct path_graph *guide, const struct point *e)
{
	struct node *n, *c;
	int i, tdist, r = 0;
	float penalty;

	while (!r) {
		if (heap_peek(pg)->visited) {
			heap_pop(pg);
			continue;
		}

		get_adjacent(pg, pg->nodes.e + pg->heap.e[0]);
		n = pg->nodes.e + pg->heap.e[0];

		n->visited = 1;

		for (i = 0; i < 4; i++) {
			if (n->adj[i] == NULL_NODE)
				continue;
			c = pg->nodes.e + n->adj[i];

			if ((tdist = n->path_dist + 1) < c->path_dist) {
				if (guide != NULL) {
					penalty = guide_penalty(guide, c);
					c->path_dist = tdist;
					c->h_dist = penalty * ((float)(tdist) + (float)square_dist(&c->p, e));

					//L("guide penalty applied: %f, %d, %f", penalty, tdist, c->h_dist);
				} else {
					c->path_dist = tdist;
					c->h_dist = (float)(tdist + square_dist(&c->p, e));
				}
			}

			if (!c->visited)
				heap_push(pg, c);
		}

		if (n->p.x == e->x && n->p.y == e->y)
			r = 1;
		else if (pg->heap.len <= 0)
			r = 2;
	}

	return r;
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
