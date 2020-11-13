#include "posix.h"

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "shared/types/darr.h"
#include "shared/types/hash.h"
#include "shared/util/log.h"

enum hash_set {
	key_set = 1 << 1,
	val_set = 1 << 0,
};

struct hash_elem {
	size_t val, keyi;
	uint8_t set;
};

struct hash {
	struct hash_elem *e;
	struct darr *keys;
	size_t cap;
	size_t len;
#ifdef CRTS_HASH_STATS
	struct hash_stats stats;
#endif
};

struct hash *
hash_init(size_t buckets, size_t bdepth, size_t keysize)
{
	struct hash *h;

	/* assert(keysize <= HASH_MAX_KEYSIZE); */

	h = calloc(1, sizeof(struct hash));

	h->cap = buckets * bdepth;

	/* Assert hash cap is a power of 2 */
	assert(h->cap > 0 && (h->cap & (h->cap - 1)) == 0);

	h->e = calloc(h->cap, sizeof(struct hash_elem));

	h->keys = darr_init(keysize);

	//L("initialized hash: cap = %ld, keysize: %ld", h->cap, keysize);

	return h;
}

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
hash_for_each_with_keys(struct hash *h, void *ctx, hash_with_keys_iterator_func ifnc)
{
	size_t i;

	for (i = 0; i < h->cap; ++i) {
		if (!(h->e[i].set & (val_set | key_set))) {
			continue;
		}

		switch (ifnc(ctx, darr_get(h->keys, h->e[i].keyi), h->e[i].val)) {
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
	/* TODO: revisit this and see which is faster */
	memset(h->e, 0, h->cap * sizeof(struct hash_elem));
	h->len = 0;

	/*
	   size_t i;

	   for (i = 0; i < h->cap; ++i) {
	        if (h->len == 0) {
	                break;
	        }

	        if (h->e[i].set & val_set) {
	                assert(h->e[i].set & key_set);
	                h->e[i].set &= ~val_set;
	                h->e[i].val = 0;
	                h->len--;
	        }
	   }
	 */
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

	for (i = 0; i < darr_item_size(hash->keys); i++) {
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
	struct hash_elem *he, *res = NULL;
	size_t i = 0;
	size_t hvi;

	uint32_t hv = compute_hash(h, key);

#ifdef CRTS_HASH_STATS
	bool collided = false;
	uint32_t chain_depth = 0;
#endif

	for (i = 0; i < h->cap; ++i) {
		hvi = (hv + i) & (h->cap - 1);

		assert(hvi < h->cap);

		he = &h->e[hvi];

		if (!he->set || memcmp(darr_get(h->keys, he->keyi), key,
			darr_item_size(h->keys)) == 0) {
			if (he->set) {
				assert(he->set & key_set);
			}

			res = he;
			break;
		}

#ifdef CRTS_HASH_STATS
		++chain_depth;
		collided = true;
#endif
	}

#ifdef CRTS_HASH_STATS
	++((struct hash *)(h))->stats.accesses;
	((struct hash *)(h))->stats.chain_depth_sum += chain_depth;

	if (collided) {
		if (chain_depth > h->stats.max_chain_depth) {
			((struct hash *)(h))->stats.max_chain_depth = chain_depth;
		}
		++((struct hash *)(h))->stats.collisions;
	}
#endif

	return res;
}

const size_t*
hash_get(const struct hash *h, const void *key)
{
	const struct hash_elem *he;

	if ((he = walk_chain(h, key)) && (he->set & val_set)) {
		return &he->val;
	} else {
		return NULL;
	}
}

void
hash_unset(struct hash *h, const void *key)
{
	const struct hash_elem *he;

	if ((he = walk_chain(h, key)) != NULL && (he->set & val_set)) {
		((struct hash_elem *)he)->set &= ~val_set;
		h->len--;
	}

	assert(hash_get(h, key) == NULL);
}

static void
hash_grow(struct hash *h)
{
	struct hash nh = {
		.cap = h->cap * 2,
		.keys = darr_init(darr_item_size(h->keys)),
		.len = 0,
	};
	size_t i;

	nh.e = calloc(nh.cap, sizeof(struct hash_elem));

	for (i = 0; i < h->cap; ++i) {
		if (h->e[i].set & val_set) {
			hash_set(&nh, darr_get(h->keys, h->e[i].keyi), h->e[i].val);
		}
	}

	free(h->e);
	darr_destroy(h->keys);
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
		he->keyi = darr_push(h->keys, key);
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
		log_bytes(darr_get(h->keys, he->keyi), darr_item_size(h->keys));
		fprintf(stderr, " -> %ld | set %d", he->val, he->set);
		fprintf(stderr, "\n");
	}
}

#ifdef CRTS_HASH_STATS
const struct hash_stats *
hash_get_stats(const struct hash *h)
{
	return &h->stats;
}
#endif
