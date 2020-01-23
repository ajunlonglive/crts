#ifndef __PATHFIND_PGRAPH_H
#define __PATHFIND_PGRAPH_H
#include <stddef.h>
#include "types/geom.h"
#include "../../../../lib/gheap/gheap.h"

#define NULL_NODE -1

struct node {
	struct point p;

	struct point flow;
	int flow_calcd;

	int path_dist;
	float h_dist;
	int visited;

	int trav;

	int adj[4];
	int adj_calcd;
};

struct path_graph {
	struct point goal;

	struct hash *hash;

	struct {
		struct gheap_ctx ctx;
		int *e;
		size_t len;
		size_t cap;
	} heap;

	struct {
		struct node *e;
		size_t len;
		size_t cap;
	} nodes;

	struct chunks *chunks;

	int (*trav_getter)(struct path_graph *g, struct node *n);
	int res;
	int possible;
};

struct node *pgraph_lookup(const struct path_graph *g, const struct point *p);
int find_or_create_node(struct path_graph *pg, const struct point *p);
void pgraph_create(struct path_graph *pg,
	struct chunks *cnks,
	const struct point *goal,
	int (*trav_getter)(struct path_graph *g, struct node *n),
	int res);
#endif
