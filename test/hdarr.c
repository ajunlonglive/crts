#define _XOPEN_SOURCE 500

#include <assert.h>
#include <stdint.h>
#include <stdlib.h>

#include "shared/types/hdarr.h"
#include "shared/util/log.h"

#define LOOPS 0xfffff
#define INI_SIZE 2
//#define DLEN 87
#define DLEN 32
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

	srandom(SEED);

	for (i = 0; i < DLEN; ++i) {
		hdarr_set(hd, &i, &i);
	}

	intt *vp;
	for (i = 0; i < LOOPS; ++i) {
		k = random() % DLEN;

		if (random() % 2 == 0) {
			if ((vp = hdarr_get(hd, &k)) != NULL) {
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
