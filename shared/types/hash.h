#ifndef __HASH_H
#define __HASH_H

#include <stdlib.h>
#include "math/geom.h"

#define MAX_KEY_SIZE 16

struct hash_elem {
	char key[MAX_KEY_SIZE];
	const void *val;
	struct hash_elem *next;
	int initialized;
};

struct hash {
	size_t keysize;
	size_t cap;
	size_t len;
	struct {
		int collisions;
		int max_bucket_depth;
	} stats;
	struct hash_elem *e;
};

struct hash *hash_init(size_t keysize);
const void *hash_get(const struct hash *h, const void *key);
int hash_set(struct hash *h, const void *key, const void *val);
#endif
