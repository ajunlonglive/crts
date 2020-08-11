#include "posix.h"

#include <stdlib.h>
#include <string.h>

#include "shared/math/kernel_filter.h"
#include "shared/math/perlin.h"
#include "terragen/gen/filters.h"
#include "terragen/gen/gen.h"

void
tg_noise(struct terragen_ctx *ctx)
{
	uint32_t rx, ry;

	for (ry = 0; ry < ctx->terra.height; ++ry) {
		for (rx = 0; rx < ctx->terra.width; ++rx) {
			struct terrain_pixel *tp = get_terrain_pix(ctx, rx, ry);

			tp->elev += perlin_two(rx, ry, ctx->opts.final_noise_amp,
				ctx->opts.final_noise_octs,
				ctx->opts.final_noise_freq,
				ctx->opts.final_noise_lacu);
		}
	}
}

void
tg_blur(struct terragen_ctx *ctx, float sigma, uint8_t r, uint8_t off, uint8_t depth)
{
	float *grid = calloc(ctx->terra.height * ctx->terra.width,
		sizeof(float) * depth),
	      kernel[r];
	uint32_t rx, ry;

	for (ry = 0; ry < ctx->opts.height; ++ry) {
		for (rx = 0; rx < ctx->opts.width; ++rx) {
			memcpy(&grid[(ry * ctx->opts.width + rx) * depth],
				get_terrain_pix(ctx, rx, ry) + off,
				sizeof(float) * depth);
		}
	}

	gen_gaussian_kernel(kernel, sigma, r);
	convolve_seperable_kernel(grid, ctx->terra.height, ctx->terra.width,
		depth, kernel, r);

	for (ry = 0; ry < ctx->opts.height; ++ry) {
		for (rx = 0; rx < ctx->opts.width; ++rx) {
			memcpy(get_terrain_pix(ctx, rx, ry) + off,
				&grid[(ry * ctx->opts.width + rx) * depth],
				sizeof(float) * depth);
		}
	}

	free(grid);
}
