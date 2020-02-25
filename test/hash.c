#include <stdint.h>
#include <stdio.h>

#include "shared/types/hash.h"

int
main(int argc, const char **argv)
{
	size_t i = 0;
	struct hash *h = hash_init(2, 1, sizeof(size_t));

	printf("setting %ld\n", i);
	hash_set(h, &i, i);
	i = 1;
	printf("setting %ld\n", i);
	hash_set(h, &i, i);
	for (i = 1; i < 99;) {
		printf("unsetting %ld\n", i);
		hash_unset(h, &i);
		i++;
		printf("setting %ld, ", i);
		hash_set(h, &i, i);
		printf("got: %lu", *hash_get(h, &i));
	}
	hash_destroy(h);

	return 0;
}

