#include "log.h"
#include "geom.h"
#include "hash.h"
#include <string.h>

#define HASH_STEP 2048

struct hash *hash_init(size_t keysize)
{
	struct hash *h;

	h = malloc(sizeof(struct hash));
	h->len = 0;
	h->cap = HASH_STEP;
	h->e = calloc(HASH_STEP, sizeof(struct hash));
	h->keysize = keysize;

	return h;
};

void *hash_get(const struct hash *h, long key)
{
	return h->e[key].val;
}

int hash_set(struct hash *h, long k, void *key, void *val)
{
	int r = 0;
	struct hash_elem *he = &h->e[k];

	if (he->key != NULL && memcmp(he->key, key, h->keysize) != 0) {
		do
			he = he->next;
		while (he != NULL);

		he = malloc(sizeof(struct hash_elem));
		memset(he, 0, sizeof(struct hash_elem));
	}

	if (he->key == NULL) {
		r = 1;
		h->len++;
		he->key = key;
	}

	he->val = val;

	return r;
}

long hash(const struct hash *hash, const void *key)
{
	const unsigned char *p = key;
	unsigned h = 16777551;
	size_t i;

	for (i = 0; i < hash->keysize; i++)
		h ^= (h << 5) + (h >> 2) + p[i];

	return h % hash->cap;
}
