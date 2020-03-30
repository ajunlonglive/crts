#include <string.h>

#include "server/sim/pathfind/pg_node.h"
#include "server/sim/pathfind/pgraph.h"
#include "server/sim/terrain.h"
#include "server/sim/terrain.h"
#include "shared/types/hdarr.h"
#include "shared/util/log.h"
#include "shared/util/mem.h"

void
pgn_summon_adj(struct pgraph *pg, struct pg_node *n)
{
	size_t i, id = n->index;
	const struct pg_node *pn;

	if (n->info & ni_adj_calcd) {
		return;
	}

	//   3
	// 1 n 0
	//   2

	size_t anodes[4];
	const struct point *s = &n->p, ps[] = {
		{ s->x + 1, s->y     },
		{ s->x - 1, s->y     },
		{ s->x,     s->y + 1 },
		{ s->x,     s->y - 1 },
	};

	for (i = 0; i < 4; i++) {
		pn = pgn_summon(pg, &ps[i], id);
		anodes[i] = pn->index;
	}

	n = hdarr_get_by_i(pg->nodes, id);

	memcpy(n->adj, anodes, sizeof(size_t) * 4);
	n->info |= ni_adj_calcd;
}

struct pg_node *
pgn_summon(struct pgraph *pg, const struct point *p, size_t parent_index)
{
	struct pg_node n, *np;
	size_t i;

	if ((np = hdarr_get(pg->nodes, p)) == NULL) {
		memset(&n, 0, sizeof(struct pg_node));

		n.p = *p;
		n.parent = parent_index;
		n.path_dist = UINT16_MAX;
		n.h_dist = UINT32_MAX;
		n.info = is_traversable(pg->chunks, &n.p, pg->et) ? ni_traversable : 0;

		i = hdarr_set(pg->nodes, p, &n);
		np = hdarr_get(pg->nodes, p);
		np->index = i;
	}

	return np;
}
