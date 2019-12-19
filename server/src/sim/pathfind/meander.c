#define _XOPEN_SOURCE 500
#include <stdlib.h>

#include "util/log.h"
#include "mapping.h"
#include "meander.h"

void meander(struct path_graph *pg, struct point *pos)
{

	int off = find_or_create_node(pg, pos);

	get_adjacent(pg, pg->nodes.e + off);

	struct node *n = pg->nodes.e + off;

	if (n->adj[off = (random() % 4)] == NULL_NODE)
		return;

	switch (off) {
	case 0:
		pos->x += 1;
		break;
	case 1:
		pos->x -= 1;
		break;
	case 2:
		pos->y -= 1;
		break;
	case 3:
		pos->y += 1;
		break;
	}
}
