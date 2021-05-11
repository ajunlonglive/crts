#include "posix.h"

#include <string.h>

#include "shared/serialize/base.h"
#include "shared/serialize/chunk.h"
#include "shared/serialize/limits.h"
#include "shared/util/log.h"

void
fill_ser_chunk(struct ser_chunk *sck, const struct chunk *ck)
{
	*sck = (struct ser_chunk){ .cp = ck->pos };

	memcpy(sck->heights, ck->heights,
		sizeof(float) * CHUNK_SIZE * CHUNK_SIZE);

	memcpy(sck->tiles, ck->tiles,
		sizeof(uint8_t) * CHUNK_SIZE * CHUNK_SIZE);
}

void
unfill_ser_chunk(const struct ser_chunk *sck, struct chunk *ck)
{
	*ck = (struct chunk){ .pos = sck->cp };

	memcpy(ck->heights, sck->heights,
		sizeof(float) * CHUNK_SIZE * CHUNK_SIZE);

	memcpy(ck->tiles, sck->tiles,
		sizeof(uint8_t) * CHUNK_SIZE * CHUNK_SIZE);
}

#define RUN_MAX 64

void
pack_ser_chunk(struct ac_coder *cod, const struct ser_chunk *sck)
{
	uint32_t i;
	enum tile t;
	uint32_t run_len, run_total = 0;

	pack_point(cod, &sck->cp, MAX_COORD, 0, CHUNK_SIZE);

	cod->lim = STEPS;
	for (i = 0; i < CHUNK_SIZE * CHUNK_SIZE; ++i) {
		ac_pack(cod, quantizef(sck->heights[i],
			MIN_HEIGHT, MAX_HEIGHT, STEPS));
	}

	t = sck->tiles[0];
	run_len = 0;
	for (i = 0; i < CHUNK_SIZE * CHUNK_SIZE; ++i) {
		if (sck->tiles[i] == t && run_len + 1 < RUN_MAX) {
			++run_len;
		} else {
			cod->lim = tile_count;
			ac_pack(cod, t);

			cod->lim = RUN_MAX;
			assert(run_len);
			ac_pack(cod, run_len);

			run_total += run_len;
			/* L(log_misc, "packed run of %d, len: %d, total: %d", t, run_len, run_total); */

			t = sck->tiles[i];
			run_len = 1;
		}
	}

	cod->lim = tile_count;
	ac_pack(cod, t);

	cod->lim = RUN_MAX;
	assert(run_len);
	ac_pack(cod, run_len);

	run_total += run_len;
	/* L(log_misc, "packed run of %d, len: %d, total: %d", t, run_len, run_total); */
	assert(run_total == 256);
}

void
unpack_ser_chunk(struct ac_decoder *dec, struct ser_chunk *sck)
{
	uint32_t i, j, v[CHUNK_SIZE * CHUNK_SIZE];

	unpack_point(dec, &sck->cp, MAX_COORD, 0, CHUNK_SIZE);

	dec->lim = STEPS;
	ac_unpack(dec, v, CHUNK_SIZE * CHUNK_SIZE);
	for (i = 0; i < CHUNK_SIZE * CHUNK_SIZE; ++i) {
		sck->heights[i] =
			unquantizef(v[i], MIN_HEIGHT, MAX_HEIGHT, STEPS);
	}

	for (i = 0; i < CHUNK_SIZE * CHUNK_SIZE;) {
		dec->lim = tile_count;
		ac_unpack(dec, &v[0], 1);

		dec->lim = RUN_MAX;
		ac_unpack(dec, &v[1], 1);

		assert(v[1]);

		for (j = 0; j < v[1]; ++j) {
			sck->tiles[i] = v[0];
			++i;
		}
	}
}

size_t
pack_chunk(const struct chunk *ck, uint8_t *buf, size_t blen)
{
	struct ac_coder cod;
	struct ser_chunk sc;

	ac_pack_init(&cod, buf, blen);
	fill_ser_chunk(&sc, ck);
	pack_ser_chunk(&cod, &sc);
	ac_pack_finish(&cod);

	return ac_coder_len(&cod);
}

size_t
unpack_chunk(struct chunk *ck, const uint8_t *buf, size_t blen)
{
	struct ac_decoder dec;
	struct ser_chunk sc;

	ac_unpack_init(&dec, buf, blen);
	unpack_ser_chunk(&dec, &sc);
	unfill_ser_chunk(&sc, ck);

	return ac_decoder_len(&dec);
}
