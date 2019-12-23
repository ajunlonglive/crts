#include <string.h>

#include "util/log.h"
#include "types/hash.h"

struct hash *hash_init(size_t buckets, size_t bdepth, size_t keysize)
{
	struct hash *h;

	h = calloc(1, sizeof(struct hash));

	h->ele.cap = buckets;
	h->ele.e = calloc(h->ele.cap * bdepth, sizeof(struct hash_elem));

	h->bdepth = bdepth;
	h->keysize = keysize;

	return h;
};

static unsigned compute_hash(const struct hash *hash, const void *key)
{
	const unsigned char *p = key;
	unsigned long h = 16777619;
	size_t i;

	for (i = 0; i < hash->keysize; i++)
		h ^= (h << 5) + (h >> 2) + p[i];

	return h & (hash->ele.cap - 1);
}

static struct hash_elem *walk_chain(const struct hash *h, const void *key, long k)
{
	struct hash_elem *he;
	size_t s = 0;

	for (s = 0; s < h->bdepth; ++s) {
		he = &h->ele.e[(k * h->bdepth) + s];

		if (!(he->init & HASH_KEY_SET && memcmp(he->key, key, h->keysize) != 0))
			return he;
	}

	return NULL;
}

const struct hash_elem *hash_get(const struct hash *h, const void *key)
{
	unsigned k = compute_hash(h, key);

	return walk_chain(h, key, k);
}

void hash_unset(const struct hash *h, const void *key)
{
	unsigned k = compute_hash(h, key);

	const struct hash_elem *he;

	if ((he = walk_chain(h, key, k)) != NULL)
		((struct hash_elem *)he)->init ^= HASH_VALUE_SET;
}

void hash_set(struct hash *h, const void *key, unsigned val)
{
	struct hash_elem *he;
	long k = compute_hash(h, key);

	if ((he = walk_chain(h, key, k)) == NULL) {
		L("uh-oh bucket full for this element");
		return;
	}

	if (!(he->init & HASH_KEY_SET)) {
		memcpy(he->key, key, h->keysize);
		he->init |= HASH_KEY_SET;
	}

	he->val = val;
	he->init |= HASH_VALUE_SET;
}

