#include "posix.h"

#include <math.h>
#include <string.h>

#include "shared/math/rand.h"
#include "shared/util/log.h"

/* random functions from musl libc */

static unsigned short __seed48[7] = { 11419, 21330, 41207, 1, 55499, 58760, 58910 };

static uint64_t
__rand48_step(unsigned short *xi, unsigned short *lc)
{
	/* L("%d, %d, %d | %d, %d, %d", xi[0], xi[1], xi[2], lc[0], lc[1], lc[2]); */
	uint64_t a, x;
	x = xi[0] | (xi[1] + 0U) << 16 | (xi[2] + 0ULL) << 32;
	a = lc[0] | (lc[1] + 0U) << 16 | (lc[2] + 0ULL) << 32;
	x = a * x + lc[3];
	xi[0] = x;
	xi[1] = x >> 16;
	xi[2] = x >> 32;
	return x & 0xffffffffffffull;
}

static double
erand48(unsigned short s[3])
{
	union {
		uint64_t u;
		double f;
	} x = { 0x3ff0000000000000ULL | __rand48_step(s, __seed48 + 3) << 4 };
	return x.f - 1.0;
}

double
drand48(void)
{
	return erand48(__seed48);
}

static long
nrand48(unsigned short s[3])
{
	return __rand48_step(s, __seed48 + 3) >> 17;
}

long
lrand48(void)
{
	return nrand48(__seed48);
}

static unsigned short *
seed48(unsigned short *s)
{
	static unsigned short p[3];
	memcpy(p, __seed48, sizeof p);
	memcpy(__seed48, s, sizeof p);
	return p;
}

void
rand_set_seed(uint32_t seed)
{
	LOG_D("seeding PRNG with %d", seed);
	seed48((unsigned short [3]){ 0x330e, seed, seed >> 16 });
}

uint32_t
rand_uniform(uint32_t range)
{
	uint32_t i = (uint32_t)floorf(drand48() * (float)range);
	/* L("rand: %f, %d (%d)", drand48(), i, range); */
	return i;
}

bool
rand_chance(uint32_t x)
{
	return x == 0 ? 0 : rand_uniform(x) == 0;
}
