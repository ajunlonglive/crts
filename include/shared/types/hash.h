#ifndef TYPES_HASH_H
#define TYPES_HASH_H

#include <stddef.h>
#include <stdint.h>

struct hash;

struct hash *hash_init(size_t buckets, size_t bdepth, size_t keysize);
void hash_destroy(struct hash *h);
const uint16_t *hash_get(const struct hash *h, const void *key);
void hash_unset(const struct hash *h, const void *key);
void hash_set(struct hash *h, const void *key, unsigned val);
#endif
