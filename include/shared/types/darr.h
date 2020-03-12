#ifndef UTIL_DARR_H
#define UTIL_DARR_H

#include <stddef.h>
#include "shared/types/iterator.h"

struct darr;

size_t darr_push(struct darr *da, const void *item);
struct darr *darr_init(size_t item_size);
void *darr_get(const struct darr *da, size_t i);
void darr_del(struct darr *da, size_t i);
void darr_destroy(struct darr *da);
void darr_for_each(struct darr *da, void *ctx, iterator_func ifnc);
void darr_set(struct darr *da, size_t i, const void *item);
size_t darr_len(const struct darr *da);
void darr_clear(struct darr *da);

size_t darr_item_size(const struct darr *da);
void *darr_raw_memory(const struct darr *da);
char *darr_point_at(const struct darr *da, size_t i);
void *darr_get_mem(struct darr *da);
#endif
