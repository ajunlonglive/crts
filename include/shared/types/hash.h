#ifndef __HASH_H
#define __HASH_H

#include <stdlib.h>
#include "shared/math/geom.h"

#define HASH_VALUE_SET 0x2
#define HASH_KEY_SET   0x4

struct hash_elem {
	char key[16];
	size_t next;
	unsigned val;
	char init;
};

struct hash {
	struct hash_elem *e;
	size_t cap;
	size_t inserted;
	size_t keysize;

#ifdef HASH_STATS
	size_t worst_lookup;
	size_t collisions;
#endif
};

struct hash *hash_init(size_t buckets, size_t bdepth, size_t keysize);
void hash_destroy(struct hash *h);
const struct hash_elem *hash_get(const struct hash *h, const void *key);
void hash_unset(const struct hash *h, const void *key);
void hash_set(struct hash *h, const void *key, unsigned val);
#endif
