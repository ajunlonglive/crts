#include "posix.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "shared/math/rand.h"

static void
write_tga_hdr(FILE *f, uint32_t height, uint32_t width)
{
	uint8_t hdr[18] = { 0 };

	hdr[2]  = 2;
	hdr[12] = 255 & width;
	hdr[13] = 255 & (width >> 8);
	hdr[14] = 255 & height;
	hdr[15] = 255 & (height >> 8);
	hdr[16] = 32;
	hdr[17] = 32;

	fwrite(hdr, 1, 18, f);
}

int
main(int argc, const char *argv[])
{
	uint32_t height = 1024, width = 1024, i, j;
	write_tga_hdr(stdout, height, width);

	rand_set_seed(strtoul(argv[1], NULL, 10));

	for (i = 0; i < height; ++i) {
		for (j = 0; j < width; ++j) {
			double rv = drand48();
			rv = (double)rand() / (double)RAND_MAX;

			uint8_t clr[4] = {
				(sin(32 * rv) * 26.0),
				(cos(rv) * 100.0),
				(sin(7 * rv) * 256.0),
				0xff
			};

			fwrite(clr, sizeof(clr), 1, stdout);
		}
	}
}
