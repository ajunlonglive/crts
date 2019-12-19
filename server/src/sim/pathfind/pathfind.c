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

struct path_graph *tile_pg_create(struct hash *cnks, const struct point *goal)
{
	struct path_graph *pg = malloc(sizeof(struct path_graph));

	pgraph_create(pg, cnks, goal, tile_trav_getter, 1);

	return pg;
}

struct path_graph *chunk_pg_create(struct hash *cnks, const struct point *goal)
{
	struct path_graph *pg = malloc(sizeof(struct path_graph));
	struct point p = nearest_chunk(goal);

	pgraph_create(pg, cnks, &p, chunk_trav_getter, CHUNK_SIZE);

	return pg;
}

int pathfind(struct path_graph *pg, struct point *p)
{
	struct node *n;
	struct path_graph *cpg;
	struct point cpgp;
	int o;

	if (!pg->possible)
		return 2;

	if ((n = pgraph_lookup(pg, p)) == NULL) {
		L("locating node...");
		reset_graph_hdist(pg, p);

		cpgp = nearest_chunk(p);
		cpg = chunk_pg_create(pg->chunks, p);

		if (pgraph_lookup(cpg, &cpgp) == NULL)
			if (brushfire(cpg, NULL, &cpgp) > 1)
				return 2;

		if (brushfire(pg, cpg, p) > 1)
			return 2;

		n = pgraph_lookup(pg, p);
		L("done!");
	}

	o = n - pg->nodes.e;
	if (!n->flow_calcd) {
		calculate_path_vector(pg, n);
		n = pg->nodes.e + o;
	}

	if (n->flow.x == 0 && n->flow.y == 0)
		return 1;

	*p = point_add(p, &n->flow);
	return 0;
}
