#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "shared/types/darr.h"
#include "shared/util/log.h"
#include "shared/util/mem.h"

struct darr {
	size_t len;
	size_t cap;
	size_t item_size;
	char *e;
};

struct darr *
darr_init(size_t item_size)
{
	struct darr *darr;
	darr = calloc(1, sizeof(struct darr));

	assert(darr != NULL);

	assert(item_size > 0);
	darr->item_size = item_size;

	return darr;
}

char *
darr_point_at(const struct darr *da, size_t i)
{
	return da->e + (i * da->item_size);
}

size_t
darr_len(const struct darr *da)
{
	return da->len;
}

size_t
darr_item_size(const struct darr *da)
{
	return da->len;
}

void *
darr_raw_memory(const struct darr *da)
{
	return da->e;
}

void *
darr_get_mem(struct darr *da)
{
	size_t i;
	union {
		void **vp;
		char **cp;
	} cp = { .cp = &da->e };

	i = get_mem(cp.vp, da->item_size, &da->len, &da->cap);

	return darr_point_at(da, i);
}


size_t
darr_push(struct darr *da, const void *item)
{
	memcpy(darr_get_mem(da), item, da->item_size);

	return da->len - 1;
}

void *
darr_get(const struct darr *da, size_t i)
{
	VBASSERT(i < da->len, "index %ld is out of bounds %ld", i, da->len);

	return darr_point_at(da, i);
}

void
darr_set(struct darr *da, size_t i, const void *item)
{
	VBASSERT(i < da->len, "index %ld is out of bounds %ld", i, da->len);

	memcpy(darr_point_at(da, i), item, da->item_size);
}

void
darr_del(struct darr *da, size_t i)
{
	VBASSERT(i < da->len, "index %ld is out of bounds %ld", i, da->len);

	da->len--;

	if (da->len > 0 && da->len != i) {
		memmove(darr_point_at(da, i), darr_point_at(da, da->len), da->item_size);
	}
}

void
darr_destroy(struct darr *da)
{
	free(da->e);
	free(da);
}

void
darr_clear(struct darr *da)
{
	da->len = 0;
}

void
darr_for_each(struct darr *da, void *ctx, iterator_func ifnc)
{
	size_t i, len = da->len;

	for (i = 0; i < len; ++i) {
		/* TODO: assert(da->len == len) to help enfore not tampering
		 * with array during iteration
		 */

		switch (ifnc(ctx, darr_point_at(da, i))) {
		case ir_cont:
			break;
		case ir_done:
			return;
		}
	}
}

void
darr_clear_iter(struct darr *da, void *ctx, iterator_func ifnc)
{
	if (da->len == 0) {
		return;
	}

	darr_for_each(da, ctx, ifnc);
	darr_clear(da);
}
