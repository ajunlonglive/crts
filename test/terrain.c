#include "posix.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#ifndef CRTS_SERVER
#define CRTS_SERVER
#endif

#include "server/sim/terrain.h"
#include "server/worldgen/gen.h"
#include "shared/math/rand.h"
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

#define D 512

static struct worldgen_opts opts = {
	.height = 512,
	.width = 512,
	.points = 10000,
	.radius = 0.5,
	.faults = 40,
	.raindrops = 10000,
	.fault_max_len = 1000,
	.fault_valley_chance = 4,
	.fault_valley_max = 20,
	.fault_valley_mod = 10,
	.fault_mtn_mod = 20,
	.fault_valley_min = 10,
	.fault_radius_pct_extent = 0.75,
	.fault_max_ang = PI * 1.2,
	.fault_boost_decay = 0.8,
	.erosion_rate = 0.05,
	.deposition_rate = 0.04,
	.raindrop_friction = 0.9,
	.raindrop_speed = 0.15,
	.raindrop_max_iterations = 800,
	.final_noise_amp =  1.0,
	.final_noise_octs = 3,
	.final_noise_freq = 0.31,
	.final_noise_lacu = 1.4,
};

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
	gen_terrain(&chunks, &opts);

	write_tga_hdr(stdout, D + 1, D + 1);

	float height_max = -INFINITY,
	      height_min = INFINITY,
	      height_sum = 0;
	for (p.x = 0; p.x < D + 1; ++p.x) {
		for (p.y = 0; p.y < D + 1; ++p.y) {
			struct point np = nearest_chunk(&p);
			struct chunk *ck = get_chunk(&chunks, &np);
			struct point rp = point_sub(&p, &ck->pos);
			float height = ck->heights[rp.x][rp.y];
			if (height > height_max) {
				height_max = height;
			}

			if (height < height_min) {
				height_min = height;
			}

			height_sum += height;
		}
	}

	L("max height: %f - min height: %f - avg: %f", height_max, height_min,
		height_sum / (D * D));

	for (p.x = 0; p.x < D + 1; ++p.x) {
		for (p.y = 0; p.y < D + 1; ++p.y) {
			/* struct point p = { x1, y1 }; */
			struct point np = nearest_chunk(&p);
			struct chunk *ck = get_chunk(&chunks, &np);
			struct point rp = point_sub(&p, &ck->pos);
			float height = ck->heights[rp.x][rp.y],
			      scaled_height[] = {
				height / height_max,
				-height / height_min
			};
			/* enum tile t = get_height_at(&chunks, &p); */

			/* uint8_t r = floorf(((float)h / (float)6.0) * 255.0); */

			if (height < 0) {
				clr[0] = scaled_height[1] * 255;
				clr[1] = 20;
				clr[2] = 0;
				clr[3] = 255;
			} else {
				clr[0] = 0;
				clr[1] = 20;
				clr[2] = scaled_height[0] * 255;
				clr[3] = 255;
			}
			/* switch (get_tile_at(&chunks, &p)) { */
			/* case tile_wetland: */
			/* 	clr[0] = 0; clr[1] = 255; clr[2] = 0; clr[3] = 255; */
			/* 	break; */
			/* case tile_stone: */
			/* 	clr[0] = 255; clr[1] = 0; clr[2] = 0; clr[3] = 255; */
			/* 	break; */
			/* case tile_forest: */
			/* 	clr[0] = 0; clr[1] = 0; clr[2] = 255; clr[3] = 255; */
			/* 	break; */
			/* case tile_peak: */
			/* 	clr[0] = 123; clr[1] = 120; clr[2] = 20; clr[3] = 255; */
			/* 	break; */
			/* case tile_dirt: */
			/* 	clr[0] = 12; clr[1] = 244; clr[2] = 245; clr[3] = 255; */
			/* 	break; */
			/* case tile_wetland_forest: */
			/* 	clr[0] = 12; clr[1] = 244; clr[2] = 45; clr[3] = 255; */
			/* 	break; */
			/* default: */
			/* 	clr[0] = 0; clr[1] = 0; clr[2] = 0; clr[3] = 255; */
			/* 	break; */
			/* } */

			fwrite(clr, sizeof(uint8_t), 4, stdout);
		}
	}
}
