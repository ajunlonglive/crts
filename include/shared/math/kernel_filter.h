#ifndef SHARED_MATH_KERNEL_FILTER_H
#define SHARED_MATH_KERNEL_FILTER_H
#include <stdint.h>

void gen_gaussian_kernel(float *kernel, float std_dev, uint32_t diameter);
void convolve_seperable_kernel(float *grid, uint32_t height, uint32_t width, uint32_t depth,
	float *kernel, uint32_t diameter);
#endif
