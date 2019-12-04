#include <string.h>

#include "util/log.h"
#include "types/hash.h"

#define HASH_STEP 2048

struct hash *hash_init(size_t keysize)
{
	struct hash *h;

	h = malloc(sizeof(struct hash));
	memset(h, 0, sizeof(struct hash));
	h->len = 0;
	h->cap = HASH_STEP;
	h->e = calloc(HASH_STEP, sizeof(struct hash_elem));
	h->keysize = keysize;

	return h;
};

static unsigned compute_hash(const struct hash *hash, const void *key)
{
	const unsigned char *p = key;
	unsigned h = 16777551;
	size_t i;

	for (i = 0; i < hash->keysize; i++)
		h ^= (h << 5) + (h >> 2) + p[i];

	return h % hash->cap;
}

void *hash_get(const struct hash *h, void *key)
{
	unsigned k = compute_hash(h, key);

	struct hash_elem *he = &h->e[k];

	if (he->key == NULL)
		return NULL;

	while (memcmp(he->key, key, h->keysize) != 0 && he->next != NULL)
		he = he->next;

	return he->val;
}

int hash_set(struct hash *h, void *key, void *val)
{
	int r = 0;
	long k = compute_hash(h, key);
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

