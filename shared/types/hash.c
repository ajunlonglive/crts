#include "posix.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "shared/math/hash.h"
#include "shared/types/darr.h"
#include "shared/types/hash.h"
#include "shared/util/log.h"
#include "shared/util/mem.h"

#define k_empty    0x80 // 0b10000000
#define k_deleted  0xfe // 0b11111110
#define k_full(v)  !(v & (1 << 7)) // k_full = 0b0xxxxxxx

#define ASSERT_VALID_CAP(cap) assert(cap >= 8); assert((cap & (cap - 1)) == 0);

#define HASH_FUNC fnv_1a_64

#define LOAD_FACTOR 0.5f

struct hash_elem {
	uint64_t val, keyi;
};

static void
fill_meta_with_empty(struct hash *h)
{
	const uint32_t len = h->cap >> 3;
	uint64_t *e = (uint64_t *)h->meta.e;

	uint32_t i;
	for (i = 0; i < len; ++i) {
		e[i] = 9259542123273814144u;
		/* this number is just k_empty (128) 8 times:
		 * ((128 << 56) | (128 << 48) | (128 << 40) | (128 << 32)
		 * | (128 << 24) | (128 << 16) | (128 << 8) | (128)) */
	}
}

static void
prepare_table(struct hash *h)
{
	darr_grow_to(&h->meta, h->cap);
	darr_grow_to(&h->e, h->cap);

	fill_meta_with_empty(h);
}

void
hash_init_(struct hash *h, size_t cap, uint64_t keysize)
{
	ASSERT_VALID_CAP(cap);

	*h = (struct hash) {
		.cap = cap, .capm = cap - 1,
		.max_load = (size_t)((float)cap * LOAD_FACTOR)
	};
	darr_init_(&h->meta, sizeof(uint8_t));
	darr_init_(&h->e, sizeof(struct hash_elem));
	darr_init_(&h->keys, keysize);

	prepare_table(h);
}

struct hash *
hash_init(size_t cap, uint64_t keysize)
{
	struct hash *h = z_calloc(1, sizeof(struct hash));

	hash_init_(h, cap, keysize);

	return h;
}

void
hash_destroy_(struct hash *h)
{
	darr_destroy_(&h->meta);
	darr_destroy_(&h->e);
	darr_destroy_(&h->keys);
}

void
hash_destroy(struct hash *h)
{
	hash_destroy_(h);
	free(h);
}

size_t
hash_len(const struct hash *h)
{
	return h->len;
}

void
hash_for_each(struct hash *h, void *ctx, iterator_func ifnc)
{
	size_t i;

	for (i = 0; i < h->cap; ++i) {
		if (!k_full(((uint8_t *)h->meta.e)[i])) {
			continue;
		}


		switch (ifnc(ctx, &((struct hash_elem *)h->e.e)[i].val)) {
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
	struct hash_elem *he;

	for (i = 0; i < h->cap; ++i) {
		if (!k_full(((uint8_t *)h->meta.e)[i])) {
			continue;
		}

		he = &((struct hash_elem *)h->e.e)[i];

		switch (ifnc(ctx, h->keys.e + he->keyi * h->keys.item_size, he->val)) {
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
	h->len = h->load = 0;
	/* maybe only this is necessarry? */
	fill_meta_with_empty(h);
}

#define match ((meta & 0x7f) == h2 \
	       && memcmp(h->keys.e + (h->keys.item_size * he->keyi), key, h->keys.item_size) == 0)

static void
probe(const struct hash *h, const void *key, struct hash_elem **ret_he, uint8_t **ret_meta, uint64_t *hv)
{
	struct hash_elem *he;
	*hv = HASH_FUNC(h->keys.item_size, key);
	const uint64_t h1 = *hv >> 7, h2 = *hv & 0x7f;
	uint8_t meta;
	uint64_t hvi = h1 & h->capm;

	meta = ((uint8_t *)h->meta.e)[hvi];
	he = &((struct hash_elem *)h->e.e)[hvi];

	while (meta == k_deleted || (k_full(meta) && !match)) {
		hvi = (hvi + 1) & h->capm;
		meta = ((uint8_t *)h->meta.e)[hvi];
		he = &((struct hash_elem *)h->e.e)[hvi];
	}

	*ret_meta = &((uint8_t *)h->meta.e)[hvi];
	*ret_he = he;
}

static void
resize(struct hash *h, size_t newcap)
{
	ASSERT_VALID_CAP(newcap);
	assert(h->len <= newcap);

	L("resizing hash from %ld to %ld", h->cap, newcap);

	uint32_t i;
	struct hash_elem *ohe, *he;
	uint64_t hv;
	uint8_t *meta;
	void *key;

	struct hash newh = (struct hash) {
		.cap = newcap, .capm = newcap - 1, .keys = h->keys,
		.len = h->len, .load = h->load,
		.max_load = (size_t)((float)newcap * LOAD_FACTOR),
	};

	darr_init_(&newh.meta, sizeof(uint8_t));
	darr_init_(&newh.e, sizeof(struct hash_elem));
	prepare_table(&newh);

	for (i = 0; i < h->cap; ++i) {
		if (!k_full(((uint8_t *)h->meta.e)[i])) {
			continue;
		}

		ohe = &((struct hash_elem *)h->e.e)[i];
		key = h->keys.e + (ohe->keyi * h->keys.item_size);

		probe(&newh, key, &he, &meta, &hv);

		assert(!k_full(*meta));

		*he = *ohe;
		*meta = hv & 0x7f;
	}

	darr_destroy_(&h->meta);
	darr_destroy_(&h->e);
	*h = newh;
}

const uint64_t*
hash_get(const struct hash *h, const void *key)
{
	struct hash_elem *he;
	uint64_t hv;
	uint8_t *meta;

	probe(h, key, &he, &meta, &hv);

	return k_full(*meta) ? &he->val : NULL;
}

void
hash_unset(struct hash *h, const void *key)
{
	struct hash_elem *he;
	uint64_t hv;
	uint8_t *meta;

	probe(h, key, &he, &meta, &hv);

	if (k_full(*meta)) {
		*meta = k_deleted;
		--h->len;
	}

	assert(hash_get(h, key) == NULL);
}

void
hash_set(struct hash *h, const void *key, uint64_t val)
{
	if (h->load > h->max_load) {
		resize(h, h->cap << 1);
	}

	struct hash_elem *he;
	uint64_t hv;
	uint8_t *meta;

	probe(h, key, &he, &meta, &hv);

	if (k_full(*meta)) {
		he->val = val;
	} else {
		he->keyi = darr_push(&h->keys, key);
		he->val = val;
		*meta = hv & 0x7f;
		++h->len;
		++h->load;
	}
}
