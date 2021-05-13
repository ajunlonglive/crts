#include "posix.h"

#include <assert.h>

#include "shared/math/rand.h"
#include "shared/types/bheap.h"
#include "shared/util/log.h"

struct val {
	uint32_t key;
	int x, y;
};

static bool
val_eql(struct val *a, struct val *b)
{
	return a->key == b->key && a->x == b->x && a->y == b->y;
}

static bool
test_swap(struct darr *bh, uint32_t i, uint32_t j)
{
	struct val a = *(struct val *)darr_get(bh, i), b = *(struct val *)darr_get(bh, j);
	darr_swap(bh, i, j);
	struct val *c = darr_get(bh, j), *d = darr_get(bh, i);

	assert(val_eql(&a, c));
	assert(val_eql(&b, d));

	return val_eql(&a, c) && val_eql(&b, d);
}

static uint32_t
naive_min(struct darr *bh)
{
	uint32_t len = darr_len(bh), i, min = UINT32_MAX;

	for (i = 0; i < len; ++i) {
		uint32_t v = *(uint32_t *)darr_get(bh, i);

		if (v < min) {
			min = v;
		}
	}

	return min;

}

void
print_heap(struct darr *bh)
{
	uint32_t len = darr_len(bh), i;

	for (i = 0; i < len; ++i) {
		uint32_t v = *(uint32_t *)darr_get(bh, i);
		printf("%d ", v);
	}

	printf("\n");
}

#define LEN 512

int
main(int argc, const char *argv[])
{
	log_init();
	log_set_lvl(log_debug);

	rand_set_seed(1);

	struct darr bh = { 0 };
	darr_init(&bh, sizeof(struct val));

	uint32_t i;
	for (i = 0; i < LEN; ++i) {
		struct val tmp = { rand_uniform(64), rand_uniform(64),
				   rand_uniform(64) };

		darr_push(&bh, &tmp);
	}

	for (i = 7; i < LEN; ++i) {
		test_swap(&bh, i, i - 7);
	}

	bheap_heapify(&bh);

	for (i = 0; i < 500; ++i) {
		struct val tmp =  { rand_uniform(256), i % 3, i + 9 };

		bheap_push(&bh, &tmp);
	}

	uint32_t len = darr_len(&bh), min;

	for (i = 0; i < len - 1; ++i) {
		struct val *c = darr_get(&bh, 0);
		min = naive_min(&bh);

		assert(c->key == min);
		if (c->key != min) {
			return 1;
		}

		bheap_pop(&bh);
	}
}
