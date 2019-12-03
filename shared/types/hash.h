#ifndef __HASH_H
#define __HASH_H

#include <stdlib.h>
#include "math/geom.h"

struct hash_elem {
	void *key;
	void *val;
	struct hash_elem *next;
};

struct hash {
	size_t keysize;
	size_t cap;
	size_t len;
	struct hash_elem *e;
};

struct hash *hash_init(size_t keysize);
void *hash_get(const struct hash *h, void *key);
int hash_set(struct hash *h, void *key, void *val);
long hash(const struct hash *h, const void *p);
#endif
