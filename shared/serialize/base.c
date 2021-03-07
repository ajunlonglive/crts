#include "posix.h"

#include <assert.h>

#include "shared/serialize/base.h"

uint32_t
quantizef(float val, float min, float max, float steps)
{
	if (val >= max) {
		val = max;
	} else if (val <= min) {
		val = min;
	}

	float scaled = (val - min) / (max - min);
	assert(scaled <= 1.0f && scaled >= 0.0f);
	uint32_t r = (uint32_t)(scaled * (steps - 1) + 0.5f);
	return r  > steps - 1 ? steps - 1 : r;
}

float
unquantizef(uint32_t val, float min, float max, float steps)
{
	return ((float)val * (1.0f / steps) * (max - min)) + min;
}

void
pack_point(struct ac_coder *cod, const struct point *p, uint16_t max,
	int16_t base, int16_t mul)
{
	cod->lim = max / mul;

	int32_t x = p->x - base, y = p->y - base;

	assert(0 <= x && x < max && 0 <= y && y < max);

	ac_pack(cod, x / mul);
	ac_pack(cod, y / mul);
}

void
unpack_point(struct ac_decoder *dec, struct point *p, uint16_t max,
	int16_t base, int16_t mul)
{
	dec->lim = max / mul;
	uint32_t v[2];

	ac_unpack(dec, v, 2);

	p->x = (v[0] + base) * mul;
	p->y = (v[1] + base) * mul;
}

void
pack_rectangle(struct ac_coder *cod, const struct rectangle *r, uint16_t max,
	int16_t base, int16_t mul, uint8_t maxl)
{
	pack_point(cod, &r->pos, max, base, mul);

	assert(r->height < maxl && r->width < maxl);

	cod->lim = maxl;
	ac_pack(cod, r->height);
	ac_pack(cod, r->width);
}

void
unpack_rectangle(struct ac_decoder *dec, struct rectangle *r, uint16_t max,
	int16_t base, int16_t mul, uint8_t maxl)
{
	unpack_point(dec, &r->pos, max, base, mul);

	dec->lim = maxl;
	uint32_t v[2];
	ac_unpack(dec, v, 2);

	r->height = v[0];
	r->width = v[1];
}
