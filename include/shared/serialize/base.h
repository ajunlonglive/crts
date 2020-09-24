#ifndef SHARED_SERIALIZE_BASE_H
#define SHARED_SERIALIZE_BASE_H

#include <stdint.h>

#include "shared/serialize/coder.h"
#include "shared/types/geom.h"

uint32_t quantizef(float val, float min, float max, float steps);
float unquantizef(uint32_t val, float min, float max, float steps);
void pack_point(struct ac_coder *cod, const struct point *p, uint16_t max,
	int16_t base, int16_t mul);
void unpack_point(struct ac_decoder *dec, struct point *p, uint16_t max,
	int16_t base, int16_t mul);
void pack_rectangle(struct ac_coder *cod, const struct rectangle *r, uint16_t max,
	int16_t base, int16_t mul, uint8_t maxl);
void unpack_rectangle(struct ac_decoder *dec, struct rectangle *r, uint16_t max,
	int16_t base, int16_t mul, uint8_t maxl);
#endif
