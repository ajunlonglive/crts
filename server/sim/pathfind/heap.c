#include "server/sim/pathfind/heap.h"
#include "server/sim/pathfind/pg_node.h"
#include "shared/util/mem.h"

static int
heap_compare(const void *const ctx, const void *const a, const void *const b)
{
	const struct pgraph *g = ctx;
	const struct pg_node *na, *nb;

	na = g->nodes.e + *(size_t *)a;
	nb = g->nodes.e + *(size_t *)b;

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
	gheap_make_heap(&pg->heap.ctx, pg->heap.e, pg->heap.len);
	gheap_sort_heap(&pg->heap.ctx, pg->heap.e, pg->heap.len);
}

size_t
heap_push(struct pgraph *pg, const struct pg_node *n)
{
	union {
		void **vp;
		size_t **ip;
	} ints = { .ip = &pg->heap.e };

	size_t off = get_mem(ints.vp, sizeof(size_t), &pg->heap.len, &pg->heap.cap);
	size_t *ip = off + pg->heap.e;

	*ip = n - pg->nodes.e;

	return *ip;
}

size_t
heap_pop(struct pgraph *pg)
{
	if (pg->heap.len <= 0) {
		return 0;
	}

	pg->heap.len--;
	pg->heap.e[0] = pg->heap.e[pg->heap.len];

	return 0;
}

struct pg_node *
heap_peek(const struct pgraph *pg)
{
	if (pg->heap.len <= 0) {
		return NULL;
	}

	return pg->nodes.e + pg->heap.e[0];
}

void
heap_init(struct pgraph *pg)
{
	pg->heap.ctx.fanout = 2;
	pg->heap.ctx.page_chunks = 1;
	pg->heap.ctx.item_size = sizeof(size_t);
	pg->heap.ctx.less_comparer = &heap_compare;
	pg->heap.ctx.less_comparer_ctx = pg;
	pg->heap.ctx.item_mover = &heap_move;
}
