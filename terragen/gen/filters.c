#include "posix.h"

#include <stdlib.h>
#include <string.h>

#include "shared/math/kernel_filter.h"
#include "shared/math/perlin.h"
#include "terragen/gen/filters.h"
#include "terragen/gen/gen.h"

void
tg_add_noise(struct terragen_ctx *ctx)
{
	uint32_t i;

	for (i = 0; i < ctx->a; ++i) {
		struct terrain_pixel *tp = &ctx->terra.heightmap[i];

		tp->elev += perlin_two(tp->x, tp->y, 1.0f, 3, 0.03f, 7.0f) * ctx->opts[tg_noise].f;
	}
}

void
tg_blur(struct terragen_ctx *ctx, float sigma, uint8_t r, uint8_t off, uint8_t depth)
{
	float *grid = calloc(ctx->a, sizeof(float) * depth),
	      kernel[r];
	uint32_t rx, ry;

	for (ry = 0; ry < ctx->opts[tg_dim].u; ++ry) {
		for (rx = 0; rx < ctx->opts[tg_dim].u; ++rx) {
			memcpy(&grid[(ry * ctx->opts[tg_dim].u + rx) * depth],
				get_terrain_pix(ctx, rx, ry) + off,
				sizeof(float) * depth);
		}
	}

	gen_gaussian_kernel(kernel, sigma, r);
	convolve_seperable_kernel(grid, ctx->opts[tg_dim].u, ctx->opts[tg_dim].u,
		depth, kernel, r);

	for (ry = 0; ry < ctx->opts[tg_dim].u; ++ry) {
		for (rx = 0; rx < ctx->opts[tg_dim].u; ++rx) {
			memcpy(get_terrain_pix(ctx, rx, ry) + off,
				&grid[(ry * ctx->opts[tg_dim].u + rx) * depth],
				sizeof(float) * depth);
		}
	}

	free(grid);
}
