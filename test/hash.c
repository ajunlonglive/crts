#include "posix.h"

#include <stdint.h>
#include <stdio.h>

#include "shared/types/hash.h"

int
main(int argc, const char **argv)
{
	size_t i;
	struct hash *h = hash_init(2, 1, sizeof(size_t));

	for (i = 0; i < 99; i++) {
		printf("setting %ld, ", i);
		fflush(stdout);
		hash_set(h, &i, i);
		printf("got: %lu\n", *hash_get(h, &i));
	}

	hash_destroy(h);

	return 0;
}
