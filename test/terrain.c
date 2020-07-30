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
	.raindrop_friction = 0.2,
	.raindrop_speed = 0.15,
	.raindrop_max_iterations = 800,
	.final_noise_amp =  1.0,
	.final_noise_octs = 3,
	.final_noise_freq = 0.31,
	.final_noise_lacu = 1.4,
};

static void
parse_opts(int argc, char *argv[])
{
	signed char opt;

	while ((opt = getopt(argc, argv, "s:d:p:r:f:v:b:e:D:S:")) != -1) {
		switch (opt) {
		case 's':
			rand_set_seed(strtoul(optarg, NULL, 10));
			break;
		case 'd':
			opts.height = opts.width = strtoul(optarg, NULL, 10);
			break;
		case 'p':
			opts.points = strtoul(optarg, NULL, 10);
			break;
		case 'r':
			opts.raindrops = strtof(optarg, NULL);
			break;
		case 'f':
			opts.faults = strtoul(optarg, NULL, 10);
			break;
		case 'v':
			opts.fault_valley_chance = strtof(optarg, NULL);
			break;
		case 'b':
			opts.fault_boost_decay = strtof(optarg, NULL);
			break;
		case 'e':
			opts.erosion_rate = strtof(optarg, NULL);
			break;
		case 'D':
			opts.deposition_rate = strtof(optarg, NULL);
			break;
		case 'S':
			opts.raindrop_speed = strtof(optarg, NULL);
			break;
		default:
			exit(1);
		}
	}
}

int32_t
main(int argc, char *argv[])
{
	log_level = ll_debug;
	struct point p;
	uint8_t clr[4];
	struct chunks chunks, *_chunks = &chunks;
	chunks_init(&_chunks);

	parse_opts(argc, argv);

	gen_terrain(&chunks, &opts);

	write_tga_hdr(stdout, opts.width + 1, opts.height + 1);

	float height_max = -INFINITY,
	      height_min = INFINITY,
	      height_sum = 0;
	for (p.x = 0; (uint32_t)p.x < opts.width + 1; ++p.x) {
		for (p.y = 0; (uint32_t)p.y < opts.height + 1; ++p.y) {
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

	for (p.x = 0; (uint32_t)p.x < opts.width + 1; ++p.x) {
		for (p.y = 0; (uint32_t)p.y < opts.height + 1; ++p.y) {
			/* struct point p = { x1, y1 }; */
			struct point np = nearest_chunk(&p);
			struct chunk *ck = get_chunk(&chunks, &np);
			struct point rp = point_sub(&p, &ck->pos);
			float height = ck->heights[rp.x][rp.y],
			      scaled_height[] = {
				height / height_max,
				-height / height_min
			};

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

			fwrite(clr, sizeof(uint8_t), 4, stdout);
		}
	}
}
