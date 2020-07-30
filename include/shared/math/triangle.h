#ifndef SHARED_MATH_TRIANGLE_H
#define SHARED_MATH_TRIANGLE_H

#include <stddef.h>

#include "shared/math/trigraph.h"
#include "shared/types/geom.h"

typedef void ((*rasterize_tri_callback)(void *ctx, float *vd_interp, size_t vd_len, int32_t x, int32_t y));

void rasterize_tri(struct tg_tri *t, void *ctx, float vertex_data[][3], size_t vd_len,
	rasterize_tri_callback cb);
#endif
