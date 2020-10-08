#include "posix.h"

#include "shared/types/bheap.h"
#include "shared/util/log.h"

static void
up_heapify(struct darr *bh, uint32_t i)
{
	assert(i < darr_len(bh));

	uint32_t p, pv, mv;

	while (i) {
		p = (i - 1) / 2;
		pv = *(uint32_t *)darr_get(bh, p);
		mv = *(uint32_t *)darr_get(bh, i);

		if (pv > mv) {
			darr_swap(bh, i, p);
			i = p;
		} else {
			break;
		}
	}
}

static void
down_heapify(struct darr *bh, uint32_t i)
{
	uint32_t len = darr_len(bh), l, r, min = i, lv = 0, rv = 0, mv = 0;

	while (1) {
		/* L("%d", len); */
		l = (2 * i) + 1;
		r = l + 1;
		mv = *(uint32_t *)darr_get(bh, i);

		/* L("  i:%u:%u l:%u:%u r:%u:%u", min, mv, l, lv, r, rv); */

		if (l < len && (lv = *(uint32_t *)darr_get(bh, l)) < mv) {
			min = l;
			mv = lv;
		}

		if (r < len && (rv = *(uint32_t *)darr_get(bh, r)) < mv) {
			min = r;
			mv = rv;
		}

		if (min == i) {
			break;
		}

		/* L("    ->min: %u:%u", min, mv); */

		darr_swap(bh, i, min);
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
			down_heapify(bh, 0);
		}
	}
}

void
bheap_push(struct darr *bh, const void *e)
{
	up_heapify(bh, darr_push(bh, e));
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
		down_heapify(bh, i);
	}
}
