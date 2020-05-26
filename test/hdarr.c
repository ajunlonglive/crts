#include <assert.h>
#include <stdint.h>
#include <stdlib.h>

#include "shared/types/hdarr.h"
#include "shared/util/log.h"

#define DLEN 0xffff
#define INI_SIZE 2
#define LOOPS 0xfffff
#define SEED 1235

typedef uint32_t intt;

static void *
hkgetter(void *e)
{
	return e;
}

int
main(int argc, const char * const *argv)
{
	struct hdarr *hd = hdarr_init(INI_SIZE, sizeof(intt), sizeof(intt), hkgetter);
	size_t i;
	intt k;

	srand(SEED);

	for (i = 0; i < 12345; ++i) {
		k = rand() % DLEN;

		hdarr_set(hd, &k, &k);
	}

	intt *vp;
	for (i = 0; i < LOOPS; ++i) {
		if (i % 0xffff == 0) {
			L("%lx / %x", i, LOOPS);
		}

		k = rand() % DLEN;

		if (rand() % 4 == 0) {
			if ((vp = hdarr_get(hd, &k)) != NULL) {
				if (k != *vp) {
					L("failing");
				}
				assert(k == *vp);
			} else {
				hdarr_set(hd, &k, &k);

				assert(hdarr_get(hd, &k) != NULL);
			}
		} else {
			hdarr_del(hd, &k);

			assert(hdarr_get(hd, &k) == NULL);
		}
	}

	hdarr_destroy(hd);

	return 0;
}
