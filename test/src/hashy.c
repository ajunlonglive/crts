#define _XOPEN_SOURCE 600

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include "types/hash.h"

static void inspect_hash(struct hash *h)
{
	size_t i;

	printf("Inspecting hash %p\n", h);
	printf("  len: %d, cap: %d, keysize: %d\n", h->len, h->cap, h->keysize);

	for (i = 0; i < h->cap; i++)
		if (h->e[i].val != NULL)
			;
	//printf("  | %4d: found value %s\n", i, h->e[i].val);
}

#define LOOPS 1024

int main(int argc, const char **argv)
{
	struct hash *h = hash_init(sizeof(struct point));
	struct point p[LOOPS];
	const char *msg = "hello";
	int i;

	inspect_hash(h);
	srandom(time(NULL));

	for (i = 0; i < LOOPS; i++) {
		p[i].x = i;
		p[i].y = i;
		hash_set(h, &p[i], (void*)msg);
	}

	inspect_hash(h);
}
