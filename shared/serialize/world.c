#include "posix.h"

#include <assert.h>
#include <math.h>
#include <stdio.h>

#include "shared/serialize/coder.h"
#include "shared/serialize/world.h"
#include "shared/sim/chunk.h"
#include "shared/types/darr.h"
#include "shared/util/log.h"

/* terrain height limits */
#define MAX_HEIGHT 256
#define MIN_HEIGHT -128
/* terrain height resolution */
#define STEPS ((MAX_HEIGHT - MIN_HEIGHT) * 1000)

/* coordinate limits */
#define MAX_COORD 4096

#define BUFSIZE 512

static uint32_t
quantizef(float val, float min, float max, float steps)
{
	assert(val <= max && val >= min);
	float scaled = (val - min) / (max - min);
	assert(scaled <= 1.0f && scaled >= 0.0f);
	uint32_t r = (uint32_t)(scaled * (steps - 1) + 0.5f);
	return r  > steps - 1 ? steps - 1 : r;
}

static float
dequantizef(uint32_t val, float min, float max, float steps)
{
	return ((float)val * (1.0f / steps) * (max - min)) + min;
}

size_t
pack_chunk(const struct chunk *c, uint8_t *buf, uint32_t len)
{
	uint32_t i;

	assert(c->pos.x < MAX_COORD && c->pos.y < MAX_COORD);
	struct ac_coder p;
	ac_pack_init(&p, buf, len);

	p.lim = MAX_COORD / CHUNK_SIZE;
	ac_pack(&p, c->pos.x / CHUNK_SIZE);
	ac_pack(&p, c->pos.y / CHUNK_SIZE);

	p.lim = tile_count;
	for (i = 0; i < CHUNK_SIZE * CHUNK_SIZE; ++i) {
		ac_pack(&p, ((enum tile *)c->tiles)[i]);
	}

	p.lim = STEPS;
	for (i = 0; i < CHUNK_SIZE * CHUNK_SIZE; ++i) {
		float h = ((float *)c->heights)[i];

		if (isnan(h)) {
			h = 0;
		}

		if (h > MAX_HEIGHT) {
			h = MAX_HEIGHT;
		} else if (h < MIN_HEIGHT) {
			h = MIN_HEIGHT;
		}

		ac_pack(&p, quantizef(h, MIN_HEIGHT, MAX_HEIGHT, STEPS));
	}

	ac_pack_finish(&p);

	return p.bufi / 8 + (p.bufi % 8 ? 1 : 0);
}

size_t
unpack_chunk(struct chunk *c, const uint8_t *buf, uint32_t len)
{
	uint32_t i;
	uint32_t vbuf[BUFSIZE] = { 0 };
	struct ac_decoder p;
	ac_unpack_init(&p, buf, len);

	p.lim = MAX_COORD / CHUNK_SIZE;
	ac_unpack(&p, vbuf, 2);

	c->pos.x = vbuf[0] * CHUNK_SIZE;
	c->pos.y = vbuf[1] * CHUNK_SIZE;

	p.lim = tile_count;
	ac_unpack(&p, vbuf, CHUNK_SIZE * CHUNK_SIZE);

	for (i = 0; i < CHUNK_SIZE * CHUNK_SIZE; ++i) {
		((enum tile *)c->tiles)[i] = vbuf[i];
	}

	p.lim = STEPS;
	ac_unpack(&p, vbuf, CHUNK_SIZE * CHUNK_SIZE);

	for (i = 0; i < CHUNK_SIZE * CHUNK_SIZE; ++i) {
		((float *)c->heights)[i] = dequantizef(vbuf[i], MIN_HEIGHT,
			MAX_HEIGHT, STEPS);
	}

	return p.bufi / 8 + (p.bufi % 8 ? 1 : 0) - 4;
}
