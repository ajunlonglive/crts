#ifndef TERRAGEN_FILTERS_H
#define TERRAGEN_FILTERS_H
#include "terragen/gen/gen.h"

void tg_blur(struct terragen_ctx *ctx, float sigma, uint8_t r, uint8_t off, uint8_t depth);
void tg_add_noise(struct terragen_ctx *ctx);
#endif
