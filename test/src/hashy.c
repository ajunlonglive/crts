#define _XOPEN_SOURCE 600

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include "types/hash.h"
#include "util/log.h"

#define LOOPS 1024
#define BUCKETS 2048
#define BDEPTH 8

static void inspect_hash(struct hash *h)
{
	unsigned long i, j, e, ebuk, klen, vlen, coll, mcoll; //, sum = 0, max_bdepth = 0;
	struct hash_elem *he;

	ebuk = mcoll = coll = klen = vlen = 0;

	printf("hash@%p\n", h);
	printf("cap: %ld, keysize: %ld, bdepth: %ld\n",
	       (long)h->ele.cap, (long)h->keysize, (long)h->bdepth);

	for (i = 0; i < h->ele.cap; i++) {
		e = 1;
		for (j = 0; j < h->bdepth; j++) {
			he = &h->ele.e[(h->bdepth * i) + j];

			if (he->init & HASH_KEY_SET) {
				e = 0;
				if (j > 0) {
					coll++;
					if (j > mcoll)
						mcoll = j;
				}

				++klen;
				if (he->init & HASH_VALUE_SET)
					++vlen;
			}
		}
	}

	printf("mcoll: %ld, coll: %ld, kset: %ld, vset: %ld\n", mcoll + 1, coll, klen, vlen);
}

int main(int argc, const char **argv)
{
	struct hash *h = hash_init(BUCKETS, BDEPTH, sizeof(struct point));
	const struct hash_elem *he;
	struct point p;
	unsigned i, cnt_n = 0, cnt_ns = 0, cnt_wv = 0;

	srandom(time(NULL));

	for (i = 0; i < LOOPS; i++) {
		p.x = random();
		p.y = random();

		hash_set(h, &p, i);

		if ((he = hash_get(h, &p)) == NULL)
			cnt_n++;
		else if (!(he->init & HASH_VALUE_SET))
			cnt_ns++;
		else if (he->val != i)
			cnt_wv++;
	}

	L("done setting %d values,\n"
	  "%d oom, %d remained unset, %d set incorrectly",
	  LOOPS, cnt_n, cnt_ns, cnt_wv);

	inspect_hash(h);

	return 0;
}
