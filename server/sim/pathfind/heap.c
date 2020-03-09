#include "server/sim/pathfind/heap.h"
#include "server/sim/pathfind/pg_node.h"
#include "shared/util/log.h"
#include "shared/util/mem.h"

/* NOTE: This used to be a real heap, relying on a 3rd party library for the
 * actual implementation.  It is the major bottleneck for pathfinding (priority
 * queue), so I went looking to rewrite it myself.  I got such a ridiculous
 * performance boost by just naive implementation below, that I left it that
 * way.  Perhaps one day this will become a real heap.
 */

void
heap_sort(struct pgraph *pg)
{
	size_t *mem = darr_raw_memory(pg->heap);
	size_t si = 0, i, len = darr_len(pg->heap);

	uint16_t cur, smallest = UINT16_MAX;

	for (i = 0; i < len; ++i) {
		cur = ((struct pg_node *)hdarr_get_by_i(pg->nodes, mem[i]))->h_dist;
		if (cur < smallest) {
			si = i;
			smallest = cur;
		}
	}

	pg->smallest = si;
}

size_t
heap_push(struct pgraph *pg, const struct pg_node *n)
{
	return darr_push(pg->heap, &n->index);
}

size_t
heap_pop(struct pgraph *pg)
{
	size_t len;

	if ((len = darr_len(pg->heap)) > 0) {
		darr_del(pg->heap, pg->smallest);
	}

	return 0;
}

struct pg_node *
heap_peek(const struct pgraph *pg)
{
	size_t *ip;

	if (darr_len(pg->heap) > 0) {
		ip = darr_get(pg->heap, pg->smallest);
		return hdarr_get_by_i(pg->nodes, *ip);
	} else {
		return NULL;
	}
}
