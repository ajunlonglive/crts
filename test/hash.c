#include "posix.h"

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "shared/types/hash.h"
#include "shared/util/log.h"

static void
test_growth(size_t max)
{
	size_t i, *vals;
	struct hash *h = hash_init(8, sizeof(size_t));

	vals = calloc(max, sizeof(size_t));

	for (i = 0; i < max; i++) {
		vals[i] = rand();
	}

	for (i = 0; i < max; i++) {
		/* L("setting %ld, %ld", i, vals[i]); */
		hash_set(h, &i, vals[i]);
		/* L("got: %ld", *hash_get(h, &i)); */
		assert(*hash_get(h, &i) == vals[i]);
	}

	hash_destroy(h);

	free(vals);
}

static void
test_ins_del(size_t amnt, size_t max)
{

	struct hash *h = hash_init(4096, sizeof(uint32_t));
	size_t i;
	uint32_t k, deleted = 0, reinserted = 0, checked = 0;

	const uint64_t *e;

	for (i = 0; i < max; ++i) {
		k = rand() % max;

		hash_set(h, &k, k);
	}

	L("done setting");

	for (i = 0; i < amnt; ++i) {
		k = rand() % max;

		if (rand() % 2 == 0) {
			if ((e = hash_get(h, &k))) {
				assert(k == *e);
				++checked;
			} else {
				hash_set(h, &k, k);

				assert(hash_get(h, &k));
				++reinserted;
			}
		} else {
			++deleted;
			hash_unset(h, &k);

			assert(!hash_get(h, &k));
		}
	}

	L("del:%d, reins:%d, check:%d", deleted, reinserted, checked);

	hash_destroy(h);
}

int
main(int argc, const char **argv)
{
	log_init();
	log_level = ll_debug;

	test_ins_del(1000000, 100000);

	test_growth(10000000);

	return 0;
}
