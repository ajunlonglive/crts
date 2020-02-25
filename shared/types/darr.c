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

static char *
point_at(const struct darr *da, size_t i)
{
	return da->e + (i * da->item_size);
}

size_t
darr_len(const struct darr *da)
{
	return da->len;
}

void *
darr_raw_memory(const struct darr *da)
{
	return da->e;
}

size_t
darr_push(struct darr *da, const void *item)
{
	size_t i;
	union {
		void **vp;
		char **cp;
	} cp = { .cp = &da->e };

	i = get_mem(cp.vp, da->item_size, &da->len, &da->cap);

	memcpy(point_at(da, i), item, da->item_size);

	return i;
}

void *
darr_get(const struct darr *da, size_t i)
{
	assert(i < da->len);

	return point_at(da, i);
}

void
darr_set(struct darr *da, size_t i, const void *item)
{
	assert(i < da->len);

	memcpy(point_at(da, i), item, da->item_size);
}

void
darr_del(struct darr *da, size_t i)
{
	assert(i < da->len);

	da->len--;

	if (da->len > 0) {
		memmove(point_at(da, i), point_at(da, da->len), da->item_size);
	}
}

void
darr_destroy(struct darr *da)
{
	free(da->e);
	free(da);
}

void
darr_for_each(struct darr *da, void *ctx, iterator_func ifnc)
{
	size_t i;

	for (i = 0; i < da->len; ++i) {
		switch (ifnc(ctx, point_at(da, i))) {
		case ir_cont:
			break;
		case ir_done:
			return;
		}
	}
}
