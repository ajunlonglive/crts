#ifndef SHARED_TYPES_DARR_H
#define SHARED_TYPES_DARR_H

#include <stddef.h>
#include <stdint.h>

#include "shared/types/iterator.h"

struct darr {
	size_t len;
	size_t cap;
	size_t item_size;
	uint8_t *e;
};

void darr_init_(struct darr *darr, size_t item_size);
struct darr *darr_init(size_t item_size);

void darr_destroy(struct darr *da);
void darr_destroy_(struct darr *da);

size_t darr_push(struct darr *da, const void *item);
void *darr_try_get(const struct darr *da, size_t i);
void *darr_get(const struct darr *da, size_t i);
void darr_del(struct darr *da, size_t i);
void darr_for_each(struct darr *da, void *ctx, iterator_func ifnc);
void darr_set(struct darr *da, size_t i, const void *item);
size_t darr_len(const struct darr *da);
void darr_clear(struct darr *da);

size_t darr_item_size(const struct darr *da);
size_t darr_size(const struct darr *da);
void *darr_raw_memory(const struct darr *da);
uint8_t *darr_point_at(const struct darr *da, size_t i);
void *darr_get_mem(struct darr *da);
void darr_grow_to(struct darr *da, size_t size);
void darr_clear_iter(struct darr *da, void *ctx, iterator_func ifnc);

void darr_swap(struct darr *da, size_t i, size_t j);
#endif
