#include "posix.h"

#include <locale.h>
#include <stdlib.h>
#include <string.h>

#include "shared/serialize/to_disk.h"
#include "shared/util/assets.h"
#include "shared/util/file_formats/tga.h"
#include "shared/util/log.h"
#include "shared/util/log.h"

enum rgba { B, G, R,  A };

int32_t
main(int32_t argc, char * const *argv)
{
	log_init();

	if (argc < 2) {
		fprintf(stderr, "usage: snap <world.crw|->\n");
		return 1;
	}

	struct chunks chunks;
	struct chunk *ck;
	chunks_init(&chunks);

	if (!load_world_from_path(argv[1], &chunks)) {
		return 1;
	}

	uint32_t i;
	int32_t maxx = INT32_MIN, maxy = INT32_MIN, minx = INT32_MAX, miny = INT32_MAX;

	for (i = 0; i < hdarr_len(&chunks.hd); ++i) {
		ck = hdarr_get_by_i(&chunks.hd, i);

		if (ck->pos.x > maxx) {
			maxx = ck->pos.x;
		}

		if (ck->pos.x < minx) {
			minx = ck->pos.x;
		}

		if (ck->pos.y > maxy) {
			maxy = ck->pos.y;
		}

		if (ck->pos.y < miny) {
			miny = ck->pos.y;
		}
	}

	uint32_t height = maxy - miny, width = maxx - minx;

	write_tga_hdr(stdout, height, width);

	struct point p = { 0, 0 };
	struct chunk *c = get_chunk(&chunks, &p);

	for (p.y = miny; p.y < maxy; ++p.y) {
		for (p.x = minx; p.x < maxx; ++p.x) {
			if (nearest_chunk(&p).x != c->pos.x) {
				c = get_chunk_at(&chunks, &p);
			}
			struct point rp = point_sub(&p, &c->pos);

			uint8_t clr[4] = { 0x00, 0x00, 0x00, 0xff };

			switch (c->tiles[rp.x][rp.y]) {
			case tile_deep_water:
			case tile_water:
				clr[B] = 0x3f;
				break;
			case tile_mountain:
			case tile_peak:
				clr[R] = c->heights[rp.x][rp.y] / 128.0f * 100;
				clr[B] = clr[G] = clr[R];
				break;
			case tile_plain:
				clr[G] = 20 + c->heights[rp.x][rp.y] / 128.0f * 100;
				break;
			case tile_wetland:
				clr[G] = 20 + c->heights[rp.x][rp.y] / 128.0f * 100;
				clr[R] = 20;
				break;
			case tile_wetland_forest:
			case tile_forest:
				clr[G] = 58;
				clr[B] = 28;
				break;
			}

			fwrite(clr, sizeof(clr), 1, stdout);
		}
	}
}
