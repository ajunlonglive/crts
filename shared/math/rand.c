#include "posix.h"

#include <stdlib.h>

#include "shared/math/rand.h"
#include "shared/util/log.h"

void
rand_set_seed(uint32_t seed)
{
	LOG_D("seeding PRNG with %d", seed);
	srand(seed);
}

uint32_t
rand_uniform(uint32_t range)
{
	uint32_t copies = RAND_MAX / range;
	uint32_t limit = range * copies;
	uint32_t r = 0;

	do {
		r = rand();
	} while (r >= limit);

	return r / copies;
}

bool
rand_chance(uint32_t x)
{
	return rand_uniform(x) == 0;
}
