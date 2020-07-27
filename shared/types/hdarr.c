#include "posix.h"

#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>

#include "shared/types/darr.h"
#include "shared/types/hash.h"
#include "shared/types/hdarr.h"
#include "shared/util/log.h"

struct hdarr {
	struct hash *hash;
	struct darr *darr;

	hdarr_key_getter kg;
};

struct hdarr *
hdarr_init(size_t size, size_t keysize, size_t item_size, hdarr_key_getter kg)
{
	struct hdarr *hd;

	hd = calloc(1, sizeof(struct hdarr));
	hd->darr = darr_init(item_size);
	hd->hash = hash_init(size, 1, keysize);
	hd->kg = kg;

	return hd;
}

struct darr *
hdarr_darr(struct hdarr *hd)
{
	return hd->darr;
}

void
hdarr_destroy(struct hdarr *hd)
{
	darr_destroy(hd->darr);
	hash_destroy(hd->hash);
	free(hd);
}

void *
hdarr_get(struct hdarr *hd, const void *key)
{
	const size_t *val;

	if ((val = hash_get(hd->hash, key)) == NULL) {
		return NULL;
	} else {
		return darr_get(hd->darr, *val);
	}
}

const size_t *
hdarr_get_i(struct hdarr *hd, const void *key)
{
	return hash_get(hd->hash, key);
}

void *
hdarr_get_by_i(struct hdarr *hd, size_t i)
{
	return darr_get(hd->darr, i);
}

void
hdarr_del(struct hdarr *hd, const void *key)
{
	const size_t *val;
	size_t len;
	const void *tailkey;

	if ((val = hash_get(hd->hash, key)) == NULL) {
		return;
	} else {
		hash_unset(hd->hash, key);

		darr_del(hd->darr, *val);

		if ((len = darr_len(hd->darr)) > 0 && len != *val) {
			tailkey = hd->kg(darr_get(hd->darr, *val));
			hash_set(hd->hash, tailkey, *val);
		}
	}
}

size_t
hdarr_set(struct hdarr *hd, const void *key, const void *value)
{
	size_t i;
	const size_t *val;

	if ((val = hash_get(hd->hash, key)) == NULL) {
		i = darr_push(hd->darr, value);

		hash_set(hd->hash, key, i);
		return i;
	} else {
		darr_set(hd->darr, *val, value);
		return *val;
	}
}

void
hdarr_reset(struct hdarr *hd, const void *okey, const void *nkey)
{
	const size_t *val;

	if ((val = hash_get(hd->hash, okey)) == NULL) {
		return;
	} else {
		hash_set(hd->hash, nkey, *val);
		hash_unset(hd->hash, okey);
	}
}

void
hdarr_clear(struct hdarr *hd)
{
	hash_clear(hd->hash);
	darr_clear(hd->darr);
}

void
hdarr_for_each(struct hdarr *hd, void *ctx, iterator_func ifnc)
{
	darr_for_each(hd->darr, ctx, ifnc);
}

size_t
hdarr_len(const struct hdarr *hd)
{
	return darr_len(hd->darr);
}
