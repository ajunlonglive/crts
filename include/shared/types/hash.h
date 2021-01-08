#ifndef TYPES_HASH_H
#define TYPES_HASH_H

#include <stddef.h>
#include <stdint.h>

#include "shared/types/darr.h"
#include "shared/types/iterator.h"

struct hash {
	struct darr meta, e, keys;
	size_t cap, len, load, max_load, capm;
};

typedef enum iteration_result ((*hash_with_keys_iterator_func)(void *ctx, void *key, uint64_t val));

uint64_t *hash_get(const struct hash *h, const void *key);

void hash_init(struct hash *h, size_t cap, uint64_t keysize);
void hash_destroy(struct hash *h);

void hash_set(struct hash *h, const void *key, uint64_t val);
void hash_unset(struct hash *h, const void *key);
void hash_clear(struct hash *h);

void hash_for_each(struct hash *h, void *ctx, iterator_func ifnc);
void hash_for_each_with_keys(struct hash *h, void *ctx, hash_with_keys_iterator_func ifnc);
#endif
