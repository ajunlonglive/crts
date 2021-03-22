#include "posix.h"

#include <stdlib.h>
#include <string.h>

#include "shared/math/kernel_filter.h"
#include "shared/math/perlin.h"
#include "shared/util/log.h"
#include "shared/util/mem.h"
#include "terragen/gen/filters.h"
#include "terragen/gen/gen.h"

void
tg_blur(struct terragen_ctx *ctx, float sigma, uint8_t diam, uint8_t off, uint8_t depth)
{
	float *grid = z_calloc(ctx->a, sizeof(float) * depth),
	      kernel[diam];
	uint32_t i;

	for (i = 0; i < ctx->a; ++i) {
		memcpy(&grid[i * depth],
			&((float *)&ctx->terra.heightmap[i])[off],
			sizeof(float) * depth);
	}

	gen_gaussian_kernel(kernel, sigma, diam);

	convolve_seperable_kernel(grid, ctx->opts[tg_dim].u, ctx->opts[tg_dim].u,
		depth, kernel, diam);

	for (i = 0; i < ctx->a; ++i) {
		memcpy(&((float *)&ctx->terra.heightmap[i])[off],
			&grid[i * depth],
			sizeof(float) * depth);
	}

	z_free(grid);
}
