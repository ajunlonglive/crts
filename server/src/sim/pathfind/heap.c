#include "heap.h"
#include "util/mem.h"

static int heap_compare(const void *const ctx, const void *const a, const void *const b)
{
	const struct path_graph *g = ctx;
	const struct node *na = g->nodes.e + *(int*)a, *nb = g->nodes.e + *(int*)b;

	return na->h_dist < nb->h_dist;
}

static void heap_move(void *const dst, const void *const src)
{
	int tmp = *(int*)dst;

	*(int*)dst = *(int*)src;
	*(int*)src = tmp;
}

void heap_sort(struct path_graph *pg)
{
	gheap_make_heap(&pg->heap.ctx, pg->heap.e, pg->heap.len);
	gheap_sort_heap(&pg->heap.ctx, pg->heap.e, pg->heap.len);
}

int heap_push(struct path_graph *pg, const struct node *n)
{
	union {
		void **vp;
		int **ip;
	} ints = { .ip = &pg->heap.e };

	int ii = get_mem(ints.vp, sizeof(int), &pg->heap.len, &pg->heap.cap);
	int *i = ii + pg->heap.e;

	*i = n - pg->nodes.e;

	heap_sort(pg);

	return *i;
}

int heap_pop(struct path_graph *pg)
{
	if (pg->heap.len <= 0)
		return -1;

	pg->heap.len--;
	pg->heap.e[0] = pg->heap.e[pg->heap.len];

	heap_sort(pg);

	return 0;
}

struct node *heap_peek(const struct path_graph *pg)
{
	if (pg->heap.len <= 0)
		return NULL;

	return pg->nodes.e + pg->heap.e[0];
}

void heap_init(struct path_graph *pg)
{
	pg->heap.ctx.fanout = 2;
	pg->heap.ctx.page_chunks = 1;
	pg->heap.ctx.item_size = sizeof(int);
	pg->heap.ctx.less_comparer = &heap_compare;
	pg->heap.ctx.less_comparer_ctx = pg;
	pg->heap.ctx.item_mover = &heap_move;
}
