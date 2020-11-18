#include "posix.h"

#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "shared/math/kernel_filter.h"
#include "shared/util/log.h"
#include "shared/util/mem.h"
#include "shared/util/util.h"

void
gen_gaussian_kernel(float *kernel, float std_dev, uint32_t diameter)
{
	uint8_t i;
	float sum = 0.0;

	for (i = 0; i < diameter; ++i) {
		float r = i - ((diameter - 1) * 0.5);
		sum += kernel[i] = expf(-(r * r) / (2 * std_dev * std_dev));
	}

	for (i = 0; i < diameter; ++i) {
		kernel[i] /= sum;
	}
}

typedef void ((*convolve_cb)(void *pixels, float *kernel, uint32_t diameter));

static float *
get_grid_item(float *grid, uint32_t height, uint32_t width, uint32_t depth,
	int32_t x, int32_t y)
{
	if (x >= (int32_t)width || x < 0 || y >= (int32_t)height || y < 0) {
		return NULL;
	}

	return &grid[x * height * depth + y * depth];
}

void
convolve_seperable_kernel(float *grid, uint32_t height, uint32_t width, uint32_t depth,
	float *kernel, uint32_t diameter)
{
	float *tmp = z_calloc(height * width * depth, sizeof(float));

	uint32_t x, y, k, l;
	const float *pix, radius = (float)(diameter - 1) * 0.5;
	float *outpix;

	for (y = 0; y < height; ++y) {
		for (x = 0; x < width; ++x) {
			outpix = get_grid_item(tmp, height, width, depth,  x, y);
			assert(outpix);

			for (k = 0; k < diameter; ++k) {
				if (!(pix = get_grid_item(grid, height, width, depth,
					x, (int32_t)y + (int32_t)(k - radius)))) {
					continue;
				}

				for (l = 0; l < depth; ++l) {
					outpix[l] += pix[l] * kernel[k];
				}
			}
		}
	}

	memset(grid, 0, sizeof(float) * height * width * depth);

	for (y = 0; y < height; ++y) {
		for (x = 0; x < width; ++x) {
			outpix = get_grid_item(grid, height, width, depth,  x, y);
			assert(outpix);

			for (k = 0; k < diameter; ++k) {
				if (!(pix = get_grid_item(tmp, height, width, depth,
					(int32_t)x + (int32_t)(k - radius), y))) {
					continue;
				}

				for (l = 0; l < depth; ++l) {
					outpix[l] += pix[l] * kernel[k];
				}
			}
		}
	}

	z_free(tmp);
}
