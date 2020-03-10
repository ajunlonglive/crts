#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "shared/types/hash.h"
#include "shared/util/log.h"

#define HASH_MAX_KEYSIZE 16

enum hash_set {
	key_set = 1 << 1,
	val_set = 1 << 0,
};

struct hash_elem {
	uint8_t key[HASH_MAX_KEYSIZE];
	size_t val;
	uint8_t set;
};

struct hash {
	struct hash_elem *e;
	size_t cap;
	size_t len;
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

	//L("initialized hash: cap = %ld, keysize: %ld", h->cap, keysize);

	return h;
};

void
hash_for_each(struct hash *h, void *ctx, iterator_func ifnc)
{
	size_t i;

	for (i = 0; i < h->cap; ++i) {
		if (!(h->e[i].set & val_set)) {
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
hash_clear(struct hash *h)
{
	size_t i;

	for (i = 0; i < h->cap; ++i) {
		if (h->len == 0) {
			break;
		}

		if (h->e[i].set & val_set) {
			h->e[i].set ^= val_set;
			h->len--;
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
	if ((he = walk_chain(h, key)) == NULL || !(he->set & val_set)) {
		return NULL;
	} else {
		return &he->val;
	}
}

void
hash_unset(struct hash *h, const void *key)
{
	const struct hash_elem *he;

	if ((he = walk_chain(h, key)) != NULL && (he->set & val_set)) {
		((struct hash_elem *)he)->set = he->set ^ val_set;
		h->len--;
	}

	assert(hash_get(h, key) == NULL);
}

static void
hash_grow(struct hash *h)
{
	struct hash nh = {
		.cap = h->cap * 2,
		.keysize = h->keysize,
		.len = 0,
	};
	size_t i;

	nh.e = calloc(nh.cap, sizeof(struct hash_elem));

	for (i = 0; i < h->cap; ++i) {
		if (h->e[i].set & key_set) {
			hash_set(&nh, h->e[i].key, h->e[i].val);
		}
	}

	free(h->e);
	*h = nh;

	L("grew hash to %ld", h->cap);
}

void
hash_set(struct hash *h, const void *key, size_t val)
{
	struct hash_elem *he;

	if ((he = walk_chain(h, key)) == NULL) {
		hash_grow(h);
		hash_set(h, key, val);
		return;
	}

	if (!(he->set & key_set)) {
		memcpy(he->key, key, h->keysize);
		he->set |= key_set;
	}

	if (!(he->set & val_set)) {
		h->len++;
		he->set |= val_set;
	}

	he->val = val;
}

size_t
hash_len(const struct hash *h)
{
	return h->len;
}

void
hash_inspect(const struct hash *h)
{
	size_t i;
	const struct hash_elem *he;

	for (i = 0; i < h->cap; ++i) {
		he = &h->e[i];

		fprintf(stderr, "%ld, ", i);
		log_bytes(he->key, h->keysize);
		fprintf(stderr, " -> %ld | set %d", he->val, he->set);
		fprintf(stderr, "\n");
	}
}
