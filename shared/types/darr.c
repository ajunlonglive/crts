#include "posix.h"

#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "shared/types/darr.h"
#include "shared/util/log.h"
#include "shared/util/mem.h"

#define DEFAULT_LEN 256

static void
ensure_mem_size(void **elem, size_t size, size_t len, size_t *cap)
{
	if (len > *cap) {
		if (*cap == 0) {
			*cap = len > DEFAULT_LEN ? len : DEFAULT_LEN;
		} else {
			*cap = *cap * 2;
		}

		*elem = z_realloc(*elem, *cap * size);
	}
}

static size_t
get_mem(void **elem, size_t size, size_t *len, size_t *cap)
{
	ensure_mem_size(elem, size, ++(*len), cap);

	return *len - 1;
}

void
darr_init(struct darr *darr, size_t item_size)
{
	assert(item_size > 0);
	*darr = (struct darr) { .item_size = item_size };
}

void
darr_destroy(struct darr *da)
{
	z_free(da->e);
}

void
darr_clear(struct darr *da)
{
	da->len = 0;
}

uint8_t *
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
	return da->item_size;
}

size_t
darr_size(const struct darr *da)
{
	return da->item_size * da->len;
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
		uint8_t **cp;
	} cp = { .cp = &da->e };

	i = get_mem(cp.vp, da->item_size, &da->len, &da->cap);

	return darr_point_at(da, i);
}

void
darr_grow_to(struct darr *da, size_t size)
{
	if (size > da->len) {
		da->len = size - 1;
		darr_get_mem(da);
	}
}

size_t
darr_push(struct darr *da, const void *item)
{
	memcpy(darr_get_mem(da), item, da->item_size);

	return da->len - 1;
}

void *
darr_try_get(const struct darr *da, size_t i)
{
	if (i < da->len) {
		return darr_point_at(da, i);
	} else {
		return NULL;
	}
}

void *
darr_get(const struct darr *da, size_t i)
{
	assert(i < da->len);

	return darr_point_at(da, i);
}

void
darr_set(struct darr *da, size_t i, const void *item)
{
	assert(i < da->len);

	memcpy(darr_point_at(da, i), item, da->item_size);
}

void
darr_del(struct darr *da, size_t i)
{
	assert(i < da->len);

	da->len--;

	if (da->len > 0 && da->len != i) {
		memmove(darr_point_at(da, i), darr_point_at(da, da->len), da->item_size);
	}
}

void
darr_for_each(struct darr *da, void *ctx, iterator_func ifnc)
{
	size_t i, len = da->len;

	for (i = 0; i < len; ++i) {
		/* help prevent tampering with array during iteration */
		assert(da->len == len);

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

void
darr_swap(struct darr *da, size_t i, size_t j)
{
	assert(i != j);

	uint8_t tmp[da->item_size];

	void *a = darr_get(da, i),
	     *b = darr_get(da, j);

	memcpy(tmp, a, da->item_size);
	darr_set(da, i, b);
	darr_set(da, j, tmp);
}
