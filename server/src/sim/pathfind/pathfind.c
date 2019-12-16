#define _DEFAULT_SOURCE

#include "../terrain.h"
#include "mapping.h"
#include "pathfind.h"
#include "sim/chunk.h"
#include "util/log.h"
#include "util/mem.h"
#include "vector_field.h"

static int chunk_trav_getter(struct path_graph *pg, struct node *n)
{
	return get_chunk(pg->chunks, &n->p)->trav;
}

static int tile_trav_getter(struct path_graph *pg, struct node *n)
{
	struct point np = nearest_chunk(&n->p), rp = point_sub(&n->p, &np);

	if (get_chunk(pg->chunks, &np)->tiles[rp.x][rp.y] <= tile_forest)
		return trav_al;
	else
		return trav_no;
}

struct hp_graph *hpgraph_create(struct hash *cnks, const struct point *goal)
{
	struct hp_graph *hpg = malloc(sizeof(struct hp_graph));

	pgraph_create(&hpg->low, cnks, goal, tile_trav_getter, 1);
	pgraph_create(&hpg->high, cnks, goal, chunk_trav_getter, CHUNK_SIZE);

	return hpg;
}

static int pg_pathfind(struct path_graph *pg, struct point *p)
{
	struct node *n;

	if ((n = pgraph_lookup(pg, p)) == NULL) {
		reset_graph_hdist(pg, p);
		while (!brushfire(pg, p));
		n = pgraph_lookup(pg, p);
	}

	if (!n->flow_calcd)
		calculate_path_vector(pg, n);

	if (n->flow.x == 0 && n->flow.y == 0)
		return 1;

	*p = point_add(p, &n->flow);
	return 0;
}

int pathfind(struct hp_graph *hpg, struct point *p)
{
	pg_pathfind(&hpg->high, p);
	pg_pathfind(&hpg->low, p);
	return 0;
}
