#include <string.h>

#include "server/sim/pathfind/pg_node.h"
#include "server/sim/pathfind/pgraph.h"
#include "server/sim/terrain.h"
#include "shared/util/mem.h"

static bool
is_traversable(struct pgraph *pg, struct pg_node *n)
{
	struct point np = nearest_chunk(&n->p), rp = point_sub(&n->p, &np);

	if (get_chunk(pg->chunks, &np)->tiles[rp.x][rp.y] <= tile_forest) {
		return true;
	} else {
		return false;
	}
}

void
pgn_summon_adj(struct pgraph *pg, struct pg_node *n)
{
	uint16_t off = n - pg->nodes.e;

	if (n->adj_calcd) {
		return;
	}

	//   3
	// 1 n 0
	//   2

	const struct point *s = &n->p, ps[] = {
		{ s->x + 1, s->y     },
		{ s->x - 1, s->y     },
		{ s->x,     s->y + 1 },
		{ s->x,     s->y - 1 },
	};

	uint16_t anodes[] = {
		pgn_summon(pg, &ps[0], n),
		pgn_summon(pg, &ps[1], n),
		pgn_summon(pg, &ps[2], n),
		pgn_summon(pg, &ps[3], n),
	};

	// reset n, since it might have been invalidated by realloc
	n = off + pg->nodes.e;

	n->adj[0] = (pg->nodes.e + anodes[0])->traversable ? anodes[0] : NULL_NODE;
	n->adj[1] = (pg->nodes.e + anodes[1])->traversable ? anodes[1] : NULL_NODE;
	n->adj[2] = (pg->nodes.e + anodes[2])->traversable ? anodes[2] : NULL_NODE;
	n->adj[3] = (pg->nodes.e + anodes[3])->traversable ? anodes[3] : NULL_NODE;

	n->adj_calcd = 1;
}

uint16_t
pgn_summon(struct pgraph *pg, const struct point *p, const struct pg_node *parent)
{
	struct pg_node *n;
	uint16_t off;

	union {
		void **vp;
		struct pg_node **np;
	} nodes = { .np = &pg->nodes.e };

	if ((n = pgraph_lookup(pg, p)) == NULL) {
		off = get_mem(nodes.vp, sizeof(struct pg_node), &pg->nodes.len,
			&pg->nodes.cap);

		assert(pg->nodes.len < UINT16_MAX);

		n = pg->nodes.e + off;
		memset(n, 0, sizeof(struct pg_node));

		n->p = *p;
		n->parent = parent;
		n->path_dist = UINT16_MAX;
		n->h_dist = UINT32_MAX;
		n->traversable = is_traversable(pg, n);

		hash_set(pg->hash, p, off);
	}

	return n - pg->nodes.e;
}

