#include "posix.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "shared/math/kernel_filter.h"
#include "shared/opengl/loaders/tga.h"
#include "shared/util/file_formats/tga.h"
#include "shared/util/log.h"
#include "shared/util/mem.h"

#define D 3

void
blur(float *src, float sigma, uint8_t diam, uint32_t h, uint32_t w)
{
	uint32_t a = h * w, i;
	float *grid = z_calloc(a * D, sizeof(float)), kernel[diam];

	for (i = 0; i < a; ++i) {
		memcpy(&grid[i * D], &src[i * D], sizeof(float) * D);
	}

	gen_gaussian_kernel(kernel, sigma, diam);

	L("%dx%dx%d", h, w, D);
	convolve_seperable_kernel(grid, h, w, D, kernel, diam);

	for (i = 0; i < a; ++i) {
		memcpy(&src[i * D], &grid[i * D], sizeof(float) * D);
	}

	z_free(grid);
}

float
float_clr(uint8_t c)
{
	return (float)c / 256.0f;
}

uint8_t
int_clr(float c)
{
	return (uint8_t)(floorf(c * 256.0f));
}

int
main(int argc, const char *argv[])
{
	log_init();
	log_level = ll_debug;

	uint16_t height, width;
	uint8_t bit_depth;

	assert(argc > 1);
	const uint8_t *img = load_tga(argv[1], &height, &width, &bit_depth);

	bit_depth /= 8;
	assert(bit_depth == D);
	L("%dx%dx%d", height, width, bit_depth);

	uint32_t a = height * width, i;
	float *fimg = z_calloc(a * 3, sizeof(float));

	for (i = 0; i < a; ++i) {
		const uint8_t *pix = &img[i * bit_depth];

		fimg[i * D]     = float_clr(pix[0]);
		fimg[i * D + 1] = float_clr(pix[1]);
		fimg[i * D + 2] = float_clr(pix[2]);
	}

	blur(fimg, 3.9f, 15, height, width);

	write_tga_hdr(stdout, height, width);

	for (i = 0; i < a; ++i) {
		uint8_t clr[4] = {
			int_clr(fimg[i * D]),
			int_clr(fimg[i * D + 1]),
			int_clr(fimg[i * D + 2]),
			0xff
		};

		fwrite(clr, sizeof(clr), 1, stdout);
	}

	z_free(fimg);
}
