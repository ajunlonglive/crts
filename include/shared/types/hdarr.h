#ifndef UTIL_HDARR_H
#define UTIL_HDARR_H

#include <stdbool.h>
#include <stddef.h>

#include "shared/types/darr.h"
#include "shared/types/hash.h"
#include "shared/types/iterator.h"

typedef const void *(*hdarr_key_getter)(void *elem);

struct hdarr {
	struct hash hash;
	struct darr darr;

	hdarr_key_getter kg;
};

void hdarr_init(struct hdarr *hd, size_t size, size_t keysize, size_t item_size, hdarr_key_getter kg);
void hdarr_destroy(struct hdarr *hd);

void hdarr_del(struct hdarr *hd, const void *key);
void *hdarr_get(const struct hdarr *hd, const void *key);
const uint64_t *hdarr_get_i(struct hdarr *hd, const void *key);
void *hdarr_get_by_i(struct hdarr *hd, size_t i);
void hdarr_for_each(struct hdarr *hd, void *ctx, iterator_func ifnc);
size_t hdarr_set(struct hdarr *hd, const void *key, const void *value);
size_t hdarr_len(const struct hdarr *hd);
void hdarr_clear(struct hdarr *hd);
#endif
