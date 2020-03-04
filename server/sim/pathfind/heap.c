#include "server/sim/pathfind/heap.h"
#include "server/sim/pathfind/pg_node.h"
#include "shared/util/log.h"
#include "shared/util/mem.h"

static int
heap_compare(const void *const ctx, const void *const a, const void *const b)
{
	const struct pgraph *g = ctx;
	const struct pg_node *na, *nb;

	na = hdarr_get_by_i(g->nodes, *(size_t *)a);
	nb = hdarr_get_by_i(g->nodes, *(size_t *)b);

	return na->h_dist < nb->h_dist;
}

static void
heap_move(void *const dst, const void *const src)
{
	size_t tmp = *(size_t *)dst;

	*(size_t *)dst = *(size_t *)src;
	*(size_t *)src = tmp;
}

void
heap_sort(struct pgraph *pg)
{
	void *mem = darr_raw_memory(pg->heap);
	size_t len = darr_len(pg->heap);

	gheap_make_heap(&pg->heap_ctx, mem, len);
	gheap_sort_heap(&pg->heap_ctx, mem, len);
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
		darr_del(pg->heap, 0);
	}

	return 0;
}

struct pg_node *
heap_peek(const struct pgraph *pg)
{
	size_t *ip;

	if (darr_len(pg->heap) > 0) {
		ip = darr_get(pg->heap, 0);
		return hdarr_get_by_i(pg->nodes, *ip);
	} else {
		return NULL;
	}
}

void
heap_init(struct pgraph *pg)
{
	pg->heap_ctx.fanout = 2;
	pg->heap_ctx.page_chunks = 1;
	pg->heap_ctx.item_size = sizeof(size_t);
	pg->heap_ctx.less_comparer = &heap_compare;
	pg->heap_ctx.less_comparer_ctx = pg;
	pg->heap_ctx.item_mover = &heap_move;
}
