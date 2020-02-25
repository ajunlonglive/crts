#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "shared/types/hash.h"
#include "shared/util/log.h"

#define HASH_MAX_KEYSIZE 16

struct hash_elem {
	uint8_t key[HASH_MAX_KEYSIZE];
	size_t next;
	size_t val;
	bool set;
};

struct hash {
	struct hash_elem *e;
	size_t cap;
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
hash_for_each(struct hash *h, void *ctx, iterator_func ifnc)
{
	size_t i;

	for (i = 0; i < h->cap; ++i) {
		if (!h->e[i].set) {
			continue;
		}


		switch (ifnc(ctx, &h->e[i].val)) {
		case ir_cont:
			break;
		case ir_done:
			return;
		}
	}
}

void
hash_destroy(struct hash *h)
{
	free(h->e);
	free(h);
}

static uint32_t
compute_hash(const struct hash *hash, const void *key)
{
	const unsigned char *p = key;
	uint32_t h = 16777619;
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

	uint32_t hv = compute_hash(h, key);

	for (i = 0; i < h->cap; ++i) {
		he = &h->e[(hv + i) & (h->cap - 1)];

		if (!he->set || memcmp(he->key, key, h->keysize) == 0) {
			return he;
		}
	}

	return NULL;
}

const size_t*
hash_get(const struct hash *h, const void *key)
{
	const struct hash_elem *he;
	if ((he = walk_chain(h, key)) == NULL || !he->set) {
		return NULL;
	} else {
		return &he->val;
	}
}

void
hash_unset(const struct hash *h, const void *key)
{
	const struct hash_elem *he;

	if ((he = walk_chain(h, key)) != NULL && he->set) {
		((struct hash_elem *)he)->set = false;
	}
}

void
hash_set(struct hash *h, const void *key, size_t val)
{
	struct hash_elem *he;

	if ((he = walk_chain(h, key)) == NULL) {
		L("hash full!");
		return;
	}

	if (!he->set) {
		memcpy(he->key, key, h->keysize);
		he->set = true;
	}

	he->val = val;
}
