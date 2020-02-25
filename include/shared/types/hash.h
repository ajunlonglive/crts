#ifndef TYPES_HASH_H
#define TYPES_HASH_H

#include <stddef.h>
#include <stdint.h>

#include "shared/types/iterator.h"

struct hash;

const size_t *hash_get(const struct hash *h, const void *key);
struct hash *hash_init(size_t buckets, size_t bdepth, size_t keysize);
void hash_destroy(struct hash *h);
void hash_for_each(struct hash *h, void *ctx, iterator_func ifnc);
void hash_set(struct hash *h, const void *key, size_t val);
void hash_unset(const struct hash *h, const void *key);
#endif
