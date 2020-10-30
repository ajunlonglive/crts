#include "posix.h"

#include <string.h>

#include "shared/types/bheap.h"
#include "shared/util/log.h"

static void
swap(uint8_t *e, uint32_t isize, uint32_t i, uint32_t j)
{
	assert(i != j);

	uint8_t tmp[isize];

	uint8_t *a = &e[isize * i],
		*b = &e[isize * j];

	memcpy(tmp, a, isize);
	memcpy(&e[isize * i], b, isize);
	memcpy(&e[isize * j], tmp, isize);
}

void
bheap_up_heapify(uint8_t *bh, uint32_t isize, uint32_t len, uint32_t i)
{
	assert(i < len);

	uint32_t p, pv, mv;

	while (i) {
		p = (i - 1) / 2;
		pv = *(uint32_t *)(bh + (isize * p));
		mv = *(uint32_t *)(bh + (isize * i));

		if (pv > mv) {
			swap(bh, isize, i, p);
			i = p;
		} else {
			break;
		}
	}
}

void
bheap_down_heapify(uint8_t *bh, uint32_t isize, uint32_t len, uint32_t i)
{
	uint32_t l, r, min = i, lv = 0, rv = 0, mv = 0;

	while (len) {
		l = (2 * i) + 1;
		r = l + 1;
		mv = *(uint32_t *)(bh + (isize * i));

		if (l < len && (lv = *(uint32_t *)(bh + (isize * l))) < mv) {
			min = l;
			mv = lv;
		}

		if (r < len && (rv = *(uint32_t *)(bh + (isize * r))) < mv) {
			min = r;
			mv = rv;
		}

		if (min == i) {
			break;
		}

		swap(bh, isize, i, min);
		i = min;
	}
}

void *
bheap_peek(struct darr *bh)
{
	return darr_get(bh, 0);
}

void
bheap_pop(struct darr *bh)
{
	if (darr_len(bh)) {
		darr_del(bh, 0);
		if (darr_len(bh)) {
			bheap_down_heapify(darr_raw_memory(bh), darr_item_size(bh),
				darr_len(bh), 0);
		}
	}
}

void
bheap_push(struct darr *bh, const void *e)
{
	uint32_t i = darr_push(bh, e);
	bheap_up_heapify(darr_raw_memory(bh), darr_item_size(bh), darr_len(bh), i);
}

void
bheap_heapify(struct darr *bh)
{
	int32_t i;
	uint32_t len = darr_len(bh);

	if (len <= 1) {
		return;
	}

	for (i = len / 2; i >= 0; --i) {
		bheap_down_heapify(darr_raw_memory(bh), darr_item_size(bh),
			darr_len(bh), i);
	}
}
