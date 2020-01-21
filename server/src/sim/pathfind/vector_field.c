#include <limits.h>
#include "mapping.h"
#include "vector_field.h"

void
calculate_path_vector(struct path_graph *pg, struct node *n)
{
	int o, i, min, mini = -1;
	struct node *c;

	o = n - pg->nodes.e;
	n->flow.x = n->flow.y = 0;
	min = INT_MAX;

	get_adjacent(pg, n);
	n = pg->nodes.e + o;

	for (i = 0; i < 4; i++) {
		if (n->adj[i] == NULL_NODE) {
			continue;
		}

		c = pg->nodes.e + n->adj[i];

		if (c->path_dist < min) {
			mini = i;
			min = c->path_dist;
		}
	}

	n->flow_calcd = 1;

	if (min <= 0 || mini < 0) {
		return;
	}

	switch (mini) {
	case 0: n->flow.x =  1; break;
	case 1: n->flow.x = -1; break;
	case 2: n->flow.y =  1; break;
	case 3: n->flow.y = -1; break;
	}
}

