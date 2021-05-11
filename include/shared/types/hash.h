#ifndef TYPES_HASH_H
#define TYPES_HASH_H

#include <stddef.h>
#include <stdint.h>

#include "shared/types/darr.h"
#include "shared/types/iterator.h"

struct hash {
	struct darr meta, e, keys;
	size_t cap, len, load, max_load, capm;
#ifndef NDEBUG
	const char *name;
	const char *file;
	const char *func;
	uint32_t line;
	bool secondary;
#endif
};

typedef enum iteration_result ((*hash_with_keys_iterator_func)(void *ctx, void *key, uint64_t val));


uint64_t *hash_get(const struct hash *h, const void *key);

#ifndef NDEBUG
#define hash_init(h, cap, keysize) \
	do { \
		_hash_init(h, cap, keysize); \
		(h)->func = __func__; \
		(h)->file = __FILE__; \
		(h)->line = __LINE__; \
		(h)->meta.name = #h ".meta"; \
		(h)->meta.func = __func__; \
		(h)->meta.file = __FILE__; \
		(h)->meta.line = __LINE__; \
		(h)->meta.secondary = true; \
		(h)->e.name = #h ".e"; \
		(h)->e.func = __func__; \
		(h)->e.file = __FILE__; \
		(h)->e.line = __LINE__; \
		(h)->e.secondary = true; \
		(h)->keys.name = #h ".keys"; \
		(h)->keys.func = __func__; \
		(h)->keys.file = __FILE__; \
		(h)->keys.line = __LINE__; \
		(h)->keys.secondary = true; \
	} while (0)
#else
#define hash_init(h, cap, keysize) _hash_init(h, cap, keysize)
#endif

void _hash_init(struct hash *h, size_t cap, uint64_t keysize);
void hash_destroy(struct hash *h);

void hash_set(struct hash *h, const void *key, uint64_t val);
void hash_unset(struct hash *h, const void *key);
void hash_clear(struct hash *h);

void hash_for_each(struct hash *h, void *ctx, iterator_func ifnc);
void hash_for_each_with_keys(struct hash *h, void *ctx, hash_with_keys_iterator_func ifnc);
#endif
