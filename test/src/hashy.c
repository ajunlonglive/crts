#define _XOPEN_SOURCE 600

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include "types/hash.h"
#include "util/log.h"

#define TEST_CASES 0xffff
#define INSERTIONS 1024
#define BUCKETS INSERTIONS * 4
#define BDEPTH 1

struct stats {
	size_t coll;
	size_t wl;
};

static struct stats inspect_hash(struct hash *h)
{
	unsigned long i, ebuk, klen, vlen; //, sum = 0, max_bdepth = 0;
	struct hash_elem *he;

	ebuk = klen = vlen = 0;

	/*
	   printf("hash@%p\n", h);
	   printf("cap: %ld, keysize: %ld\n", (long)h->cap, (long)h->keysize);
	 */

	for (i = 0; i < h->cap; i++) {
		he = &h->e[i];

		if (he->init & HASH_KEY_SET) {

			++klen;
			if (he->init & HASH_VALUE_SET)
				++vlen;
		}
	}

	/*
	   printf("kset: %ld, vset: %ld\n", klen, vlen);
	 */

	struct stats s = { 0, 0 };

#ifdef HASH_STATS
	//printf("collisions: %ld, worst lookup: %ld\n", (long)h->collisions, (long)h->worst_lookup);
	s.coll = h->collisions;
	s.wl = h->worst_lookup;
#endif

	return s;
}

static struct stats test(void)
{
	struct hash *h = hash_init(BUCKETS, BDEPTH, sizeof(struct point));
	const struct hash_elem *he;
	struct point p;
	unsigned i, cnt_n = 0, cnt_ns = 0, cnt_wv = 0;

	srandom(time(NULL));

	for (i = 0; i < INSERTIONS; i++) {
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

	if (cnt_n | cnt_ns | cnt_wv) {
		L("hash implementation is bad,\n"
		  "  %d oom, %d remained unset, %d set incorrectly",
		  cnt_n, cnt_ns, cnt_wv);
	}

	struct stats s = inspect_hash(h);
	free(h->e);
	free(h);

	return s;
};

int main(int argc, const char **argv)
{
	size_t i;
	struct stats s;
	double sc = 0, sw = 0;

	for (i = 0; i < TEST_CASES; i++) {
		s = test();
		sc += (double)s.coll;
		sw += (double)s.wl;
	}

	printf("average: %.1f collisions, %.1f worst lookup\n",
	       sc / (double)TEST_CASES, sw / (double)TEST_CASES);

	return 0;
}
