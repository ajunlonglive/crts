#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "shared/types/hash.h"
#include "shared/util/log.h"

#define HASH_MAX_KEYSIZE 16

enum hash_element_state {
	hes_empty = 0,
	hes_full  = 1,
};

struct hash_elem {
	uint8_t key[HASH_MAX_KEYSIZE];
	size_t next;
	uint16_t val;
	uint8_t init;
};

struct hash {
	struct hash_elem *e;
	size_t cap;
	size_t inserted;
	size_t keysize;
};

struct hash *
hash_init(size_t buckets, size_t bdepth, size_t keysize)
{
	struct hash *h;

	assert(keysize <= HASH_MAX_KEYSIZE);

	h = calloc(1, sizeof(struct hash));

	h->cap = buckets * bdepth;
	h->e = calloc(h->cap, sizeof(struct hash_elem));

	h->keysize = keysize;

	L("initialized hash: cap = %ld, keysize: %ld", h->cap, keysize);

	return h;
};

void
hash_destroy(struct hash *h)
{
	free(h->e);
	free(h);
}

static unsigned
compute_hash(const struct hash *hash, const void *key)
{
	const unsigned char *p = key;
	unsigned h = 16777619;
	size_t i;

	for (i = 0; i < hash->keysize; i++) {
		h ^= (h << 5) + (h >> 2) + p[i];
	}

	return h;
}

/*
 * Returns the first matching hash element, or the first empty element
 */
static struct hash_elem *
walk_chain(const struct hash *h, const void *key)
{
	struct hash_elem *he;
	size_t i = 0;

	unsigned hv = compute_hash(h, key);

	for (i = 0; i < h->cap; ++i) {
		he = &h->e[(hv + i) & (h->cap - 1)];

		if (!(he->init & hes_full) || memcmp(he->key, key, h->keysize) == 0) {
			return he;
		}
	}

	return NULL;
}

const uint16_t *
hash_get(const struct hash *h, const void *key)
{
	const struct hash_elem *he;
	if ((he = walk_chain(h, key)) == NULL || !(he->init & hes_full)) {
		return NULL;
	} else {
		return &he->val;
	}
}

void
hash_unset(const struct hash *h, const void *key)
{
	const struct hash_elem *he;

	if ((he = walk_chain(h, key)) != NULL) {
		((struct hash_elem *)he)->init ^= hes_full;
	}
}

void
hash_set(struct hash *h, const void *key, unsigned val)
{
	struct hash_elem *he;

	if ((he = walk_chain(h, key)) == NULL) {
		L("hash full!");
		return;
	}

	if (!(he->init & hes_full)) {
		memcpy(he->key, key, h->keysize);
		h->inserted++;
	}

	he->val = val;
	he->init |= hes_full;
}
