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
	struct point p[LOOPS], *pp;
	const char *msg = "hello";
	long key;
	int i;

	printf("sizeof(long) = %d, longlong = %d\n", sizeof(long), sizeof(long long));
	inspect_hash(h);
	srandom(time(NULL));

	for (i = 0; i < LOOPS; i++) {
		p[i].x = i;
		p[i].y = i;
		key = hash(h, &p[i]);
		if (hash_set(h, &p[i], (void*)msg) == 0) {
			pp = h->e[key].key;
			printf("point { %d, %d } was not inserted { %d, %d }\n", p[i].x, p[i].y, pp->x, pp->y);
		}
	}

	inspect_hash(h);
}
