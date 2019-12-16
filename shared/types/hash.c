#include <string.h>

#include "util/log.h"
#include "types/hash.h"

#define HASH_STEP 2048 * 8

struct hash *hash_init(size_t keysize)
{
	struct hash *h;

	h = malloc(sizeof(struct hash));
	memset(h, 0, sizeof(struct hash));
	h->len = 0;
	h->cap = HASH_STEP;
	h->e = calloc(HASH_STEP, sizeof(struct hash_elem));
	memset(h->e, 0, HASH_STEP * sizeof(struct hash_elem));
	h->keysize = keysize;

	h->stats.collisions = 0;
	h->stats.max_bucket_depth = 0;

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

const void *hash_get(const struct hash *h, const void *key)
{
	unsigned k = compute_hash(h, key);

	struct hash_elem *he = &h->e[k];

	if (!he->initialized)
		return NULL;

	//L("(%p %p), %p, %p, %d, %d", h, he, he->key, key, h->keysize, k);
	while (memcmp(he->key, key, h->keysize) != 0) {
		if (he->next != NULL && he->next->initialized)
			he = he->next;
		else
			return NULL;
	}

	return he->val;
}

int hash_set(struct hash *h, const void *key, const void *val)
{
	int r = 0;
	long k = compute_hash(h, key);
	struct hash_elem *he = &h->e[k];

	int bdepth = 0;

	//L("(%p %p), %p, %p, %d, %ld", h, he, he->key, key, h->keysize, k);
	if (he->initialized && memcmp(he->key, key, h->keysize) != 0) {
		while (he->next != NULL) {
			bdepth++;
			he = he->next;
		}

		if (bdepth > h->stats.max_bucket_depth)
			h->stats.max_bucket_depth = bdepth;

		he = he->next = malloc(sizeof(struct hash_elem));
		memset(he, 0, sizeof(struct hash_elem));
	}

	if (!he->initialized) {
		r = 1;
		h->len++;
		memcpy(he->key, key, h->keysize);
		if (bdepth > 0)
			h->stats.collisions++;
		he->initialized = 1;
	}

	he->val = val;

	return r;
}

