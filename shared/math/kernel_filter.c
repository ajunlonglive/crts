#include "posix.h"

#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "shared/math/kernel_filter.h"
#include "shared/util/log.h"
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

	return &grid[(y * width + x) * depth];
}

void
convolve_seperable_kernel(float *grid, uint32_t height, uint32_t width, uint32_t depth,
	float *kernel, uint32_t diameter)
{
	float *out = calloc(height * width * depth, sizeof(float));

	uint32_t i, j, k, l;
	const float *pix, radius = (float)(diameter - 1) * 0.5;
	float *outpix;

	for (i = 0; i < height; ++i) {
		for (j = 0; j < width; ++j) {
			outpix = get_grid_item(out, height, width, depth,  i, j);
			assert(outpix);

			for (k = 0; k < diameter; ++k) {
				if (!(pix = get_grid_item(grid, height, width, depth,
					i, (int32_t)j + (int32_t)(k - radius)))) {
					continue;
				}

				for (l = 0; l < depth; ++l) {
					outpix[l] += pix[l] * kernel[k];
				}
			}
		}
	}

	float *tmp = grid;
	grid = out;
	out = tmp;

	memset(out, 0, sizeof(float) * height * width * depth);

	for (i = 0; i < height; ++i) {
		for (j = 0; j < width; ++j) {
			outpix = get_grid_item(out, height, width, depth,  i, j);
			assert(outpix);

			for (k = 0; k < diameter; ++k) {
				if (!(pix = get_grid_item(grid, height, width, depth,
					i, (int32_t)j + (int32_t)(k - radius)))) {
					continue;
				}

				for (l = 0; l < depth; ++l) {
					outpix[l] += pix[l] * kernel[k];
				}
			}
		}
	}

	free(grid);
}
