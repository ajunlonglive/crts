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
	struct hash h = { 0 };
	hash_init(&h, HASH_SIZE, sizeof(size_t));

	for (i = 0; i < HASH_SIZE / 2; i++) {
		hash_set(&h, &i, i);
		assert(*hash_get(&h, &i) == i);
	}

	const struct hash_stats *stats = &h.stats;

	printf("collisions: %u, max chain depth: %u, agv: %f\n",
		(uint32_t)stats->collisions,
		(uint32_t)stats->max_chain_depth,
		stats->chain_depth_sum / ((float)stats->accesses)
		);

	hash_destroy(&h);

	return 0;
}
