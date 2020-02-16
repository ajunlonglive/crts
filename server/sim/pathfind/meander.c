#define _XOPEN_SOURCE 500
#include <stdlib.h>

#include "server/sim/pathfind/meander.h"
#include "server/sim/pathfind/pg_node.h"
#include "shared/util/log.h"

void
meander(struct pgraph *pg, struct point *pos)
{

	int off = pgn_summon(pg, pos, NULL);

	pgn_summon_adj(pg, pg->nodes.e + off);

	struct pg_node *n = pg->nodes.e + off;

	if (n->adj[off = (random() % 4)] == NULL_NODE) {
		return;
	}

	switch (off) {
	case 0:
		pos->x += 1;
		break;
	case 1:
		pos->x -= 1;
		break;
	case 2:
		pos->y += 1;
		break;
	case 3:
		pos->y -= 1;
		break;
	}
}
