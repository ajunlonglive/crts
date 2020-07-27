#include "posix.h"

#include <stdio.h>
#include <stdlib.h>

#ifndef CRTS_SERVER
#define CRTS_SERVER
#endif

#include "server/sim/gen_terrain.h"
#include "shared/math/rand.h"
#include "server/sim/terrain.h"
#include "shared/util/log.h"

static void
write_tga_hdr(FILE *tga, uint32_t width, uint32_t height)
{
	uint8_t hdr[18] = { 0 };

	hdr[2]  = 2;
	hdr[12] = 255 & width;
	hdr[13] = 255 & (width >> 8);
	hdr[14] = 255 & height;
	hdr[15] = 255 & (height >> 8);
	hdr[16] = 32;
	hdr[17] = 32;

	fwrite(hdr, 1, 18, tga);
}

#define D 256

int32_t
main(int argc, char *argv[])
{
	log_level = ll_debug;
	struct point p;
	uint8_t clr[4];
	struct chunks chunks, *_chunks = &chunks;
	chunks_init(&_chunks);

	L("%s", argv[1]);
	rand_set_seed(strtol(argv[1], NULL, 10));
	gen_terrain(&chunks, D, D, strtol(argv[2], NULL, 10));

	write_tga_hdr(stdout, D, D);

	for (p.x = 0; p.x < D; ++p.x) {
		for (p.y = 0; p.y < D; ++p.y) {
			switch (get_tile_at(&chunks, &p)) {
			case tile_wetland:
				clr[0] = 0; clr[1] = 255; clr[2] = 0; clr[3] = 255;
				break;
			case tile_stone:
				clr[0] = 255; clr[1] = 0; clr[2] = 0; clr[3] = 255;
				break;
			case tile_forest:
				clr[0] = 0; clr[1] = 0; clr[2] = 255; clr[3] = 255;
				break;
			case tile_peak:
				clr[0] = 123; clr[1] = 120; clr[2] = 20; clr[3] = 255;
				break;
			case tile_dirt:
				clr[0] = 12; clr[1] = 244; clr[2] = 245; clr[3] = 255;
				break;
			case tile_wetland_forest:
				clr[0] = 12; clr[1] = 244; clr[2] = 45; clr[3] = 255;
				break;
			default:
				clr[0] = 0; clr[1] = 0; clr[2] = 0; clr[3] = 255;
				break;
			}

			fwrite(clr, sizeof(uint8_t), 4, stdout);
		}
	}
}
