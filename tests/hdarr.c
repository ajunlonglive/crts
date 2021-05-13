#include "posix.h"

#include <assert.h>
#include <stdint.h>
#include <stdlib.h>

#include "shared/types/hdarr.h"
#include "shared/util/log.h"

typedef uint32_t intt;

static const void *
hkgetter(void *e)
{
	return e;
}

static void
stress_test(uint32_t ini_size, uint32_t loops, uint32_t dlen, uint32_t ini_set)
{
	struct hdarr hd = { 0 };
	hdarr_init(&hd, ini_size, sizeof(intt), sizeof(intt), hkgetter);
	size_t i;
	intt k;

	for (i = 0; i < ini_set; ++i) {
		k = rand() % dlen;

		hdarr_set(&hd, &k, &k);
	}

	intt *vp;
	for (i = 0; i < loops; ++i) {
		k = rand() % dlen;

		if (rand() % 4 == 0) {
			if ((vp = hdarr_get(&hd, &k))) {
				assert(k == *vp);
			} else {
				hdarr_set(&hd, &k, &k);
				assert(hdarr_get(&hd, &k));
			}
		} else {
			hdarr_del(&hd, &k);
			assert(!hdarr_get(&hd, &k));
		}
	}

	hdarr_destroy(&hd);
}

uint32_t
pow2(uint32_t n)
{
	uint32_t i, v = 1;

	for (i = 1; i < n; ++i) {
		v *= 2;
	}

	return v;
}

int
main(int argc, const char * const *argv)
{
	log_init();
	log_set_lvl(log_info);

	stress_test(16, 16, 16, 16);

	srand(12346);

	uint32_t i, a, b, c, d;
	for (i = 0; i < 1000; ++i) {
		a = pow2((rand() % 14) + 4);
		b = rand() % 100000;
		c = rand() % 100000;
		d = rand() % 100000;

		LOG_I(log_misc, "stress_test(%d, %d, %d, %d)", a, b, c, d);
		stress_test(a, b, c, d);
	}

	return 0;
}
