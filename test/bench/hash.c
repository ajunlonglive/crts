#include "posix.h"

#include <assert.h>
#include <stdint.h>
#include <stdio.h>

#include "shared/types/hash.h"
#include "shared/util/log.h"

#define HASH_SIZE (4096 * 2)

int
main(int argc, const char **argv)
{
	log_init();
	log_level = ll_debug;
	size_t i;
	struct hash *h = hash_init(HASH_SIZE, sizeof(size_t));

	for (i = 0; i < HASH_SIZE / 2; i++) {
		hash_set(h, &i, i);
		assert(*hash_get(h, &i) == i);
	}

	const struct hash_stats *stats = &h->stats;

	printf("collisions: %ld, max chain depth: %ld, agv: %f\n", stats->collisions, stats->max_chain_depth,
		stats->chain_depth_sum / ((float)stats->accesses)
		);

	hash_destroy(h);

	return 0;
}
