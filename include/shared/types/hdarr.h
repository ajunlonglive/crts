#ifndef UTIL_HDARR_H
#define UTIL_HDARR_H

#include <stddef.h>
#include "shared/types/iterator.h"

struct hdarr;

struct hdarr *hdarr_init(size_t size, size_t keysize, size_t item_size);
void *hdarr_get(struct hdarr *hd, const void *key);
void hdarr_destroy(struct hdarr *hd);
void hdarr_for_each(struct hdarr *hd, void *ctx, iterator_func ifnc);
void hdarr_set(struct hdarr *hd, const void *key, const void *value);
#endif
