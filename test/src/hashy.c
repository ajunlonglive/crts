#define _XOPEN_SOURCE 600

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <sys/time.h>
#include "types/hash.h"
#include "util/log.h"

#define TEST_CASES 0xfff
#define INSERTIONS 1024
#define BUCKETS INSERTIONS * 8
#define BDEPTH 1

struct stats {
	size_t coll;
	size_t wl;
	long long mus;
	long long mug;
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

static long elapsed()
{
	static struct timespec last = { 0, 0 };
	struct timespec this;
	long ms;

	clock_gettime(CLOCK_REALTIME, &this);

	ms = ((this.tv_sec - last.tv_sec) * 1000000) + (this.tv_nsec / 1000) - (last.tv_nsec / 1000);

	last = this;

	return ms;
}

static struct stats test(void)
{
	struct hash *h = hash_init(BUCKETS, BDEPTH, sizeof(struct point));
	const struct hash_elem *he;
	struct point p;
	unsigned i, cnt_n = 0, cnt_ns = 0, cnt_wv = 0;
	long long msa = 0, msb = 0;

	struct timespec tv_start;

	clock_gettime(CLOCK_REALTIME, &tv_start);
	srandom(tv_start.tv_nsec);

	elapsed();
	for (i = 0; i < INSERTIONS; i++) {
		p.x = random();
		p.y = random();

		hash_set(h, &p, i);
		he = hash_get(h, &p);

		if (he == NULL)
			cnt_n++;
		else if (!(he->init & HASH_VALUE_SET))
			cnt_ns++;
		else if (he->val != i)
			cnt_wv++;
	}
	msb += elapsed();

	if (cnt_n | cnt_ns | cnt_wv) {
		L("hash implementation is bad,\n"
		  "  %d oom, %d remained unset, %d set incorrectly",
		  cnt_n, cnt_ns, cnt_wv);
	}

	struct stats s = inspect_hash(h);
	s.mus = msa / INSERTIONS;
	s.mug = msb / INSERTIONS;

	free(h->e);
	free(h);

	return s;
};

int main(int argc, const char **argv)
{
	size_t i;
	struct stats s;
	long double sc = 0, sw = 0;
	long long st1 = 0, st2 = 0;

	for (i = 0; i < TEST_CASES; i++) {
		s = test();
		sc += (long double)s.coll;
		sw += (long double)s.wl;
		st1 += s.mus;
		st2 += s.mug;
	}

	printf("average: %.1Lf collisions, %.1Lf worst lookup,"
	       "set time: %lldus, lookup time: %lldus\n",
	       sc / (double)TEST_CASES, sw / (double)TEST_CASES, st1 / TEST_CASES, st2 / TEST_CASES);

	return 0;
}
