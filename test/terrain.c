#include "posix.h"

#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#ifndef CRTS_SERVER
#define CRTS_SERVER
#endif

uint8_t colors[][3] = {
	[0] = { 0, 0, 85 },
	[1] = { 0, 0, 127 },
	[14] = { 42, 42, 170 },
	[13] = { 85, 0, 170 },
	[2] = { 127, 127, 0 },
	[10] = { 127, 170, 42 },
	[11] = { 85, 127, 42 },
	[12] = { 85, 42, 42 },
	[3] = { 85, 170, 85 },
	[7] = { 127, 85, 42 },
	[8] = { 0, 170, 0 },
	[4] = { 0, 170, 0 },
	[9] = { 170, 127, 42 },
	[5] = { 237, 237, 237 },
	[6] = { 237, 237, 237 },
	[15] = { 212, 170, 42 },
	[17] = { 85, 42, 0 },
	[18] = { 107, 107, 107 },
	[16] = { 237, 237, 237 },
	[19] = { 237, 237, 237 },
	[20] = { 127, 85, 42 },
	[21] = { 0, 170, 0 },
	[22] = { 212, 0, 0 },
	[23] = { 87, 87, 87 },
};

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
print_current_opts(void)
{
	L(
		"opts = {\n"
		"       .seed = %d,\n"
		"	.height = %d,\n"
		"	.width = %d,\n"
		"	.points = %d,\n"
		"	.radius = %f,\n"
		"	.faults = %d,\n"
		"	.raindrops = %d,\n"
		"	.fault_max_len = %d,\n"
		"	.fault_valley_chance = %d,\n"
		"	.fault_valley_max = %d,\n"
		"	.fault_valley_mod = %d,\n"
		"	.fault_mtn_mod = %d,\n"
		"	.fault_valley_min = %d,\n"
		"	.fault_radius_pct_extent = %f,\n"
		"	.fault_max_ang = %f,\n"
		"	.fault_boost_decay = %f,\n"
		"	.erosion_rate = %f,\n"
		"	.deposition_rate = %f,\n"
		"	.raindrop_friction = %f,\n"
		"	.raindrop_speed = %f,\n"
		"	.raindrop_max_iterations = %d,\n"
		"	.final_noise_amp =  %f,\n"
		"	.final_noise_octs = %d\n"
		"	.final_noise_freq = %f,\n"
		"	.final_noise_lacu = %f,\n"
		"};\n",
		opts.seed,
		opts.height,
		opts.width,
		opts.points,
		opts.radius,
		opts.faults,
		opts.raindrops,
		opts.fault_max_len,
		opts.fault_valley_chance,
		opts.fault_valley_max,
		opts.fault_valley_mod,
		opts.fault_mtn_mod,
		opts.fault_valley_min,
		opts.fault_radius_pct_extent,
		opts.fault_max_ang,
		opts.fault_boost_decay,
		opts.erosion_rate,
		opts.deposition_rate,
		opts.raindrop_friction,
		opts.raindrop_speed,
		opts.raindrop_max_iterations,
		opts.final_noise_amp,
		opts.final_noise_octs,
		opts.final_noise_freq,
		opts.final_noise_lacu
		);

}

static void
parse_opts(int argc, char *argv[])
{
	signed char opt;

	while ((opt = getopt(argc, argv, "s:d:p:r:f:v:b:e:D:S:q")) != -1) {
		switch (opt) {
		case 's':
			opts.seed = strtoul(optarg, NULL, 10);
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
		case 'q':
			print_current_opts();
			exit(0);
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
			enum tile t = ck->tiles[rp.x][rp.y];
			float height = ck->heights[rp.x][rp.y],
			      scaled_height = (height - height_min)
					      / (height_max - height_min);

			/* L("scaled height: %f", scaled_height); */

			clr[0] = colors[t][2] * scaled_height;
			clr[1] = colors[t][1] * scaled_height;
			clr[2] = colors[t][0] * scaled_height;
			clr[3] = 255;

			fwrite(clr, sizeof(uint8_t), 4, stdout);
		}
	}
}
