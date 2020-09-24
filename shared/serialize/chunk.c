#include "posix.h"

#include <string.h>

#include "shared/serialize/base.h"
#include "shared/serialize/chunk.h"
#include "shared/serialize/limits.h"

void
fill_ser_chunk(struct ser_chunk *sck, const struct chunk *ck)
{
	*sck = (struct ser_chunk){ .cp = ck->pos };

	memcpy(sck->heights, ck->heights,
		sizeof(float) * CHUNK_SIZE * CHUNK_SIZE);

	memcpy(sck->tiles, ck->tiles,
		sizeof(enum tile) * CHUNK_SIZE * CHUNK_SIZE);
}

void
unfill_ser_chunk(const struct ser_chunk *sck, struct chunk *ck)
{
	*ck = (struct chunk){ .pos = sck->cp };

	memcpy(ck->heights, sck->heights,
		sizeof(float) * CHUNK_SIZE * CHUNK_SIZE);

	memcpy(ck->tiles, sck->tiles,
		sizeof(enum tile) * CHUNK_SIZE * CHUNK_SIZE);
}

void
pack_ser_chunk(struct ac_coder *cod, const struct ser_chunk *sck)
{
	uint32_t i;

	pack_point(cod, &sck->cp, MAX_COORD, 0, CHUNK_SIZE);

	cod->lim = STEPS;
	for (i = 0; i < CHUNK_SIZE * CHUNK_SIZE; ++i) {
		ac_pack(cod, quantizef(sck->heights[i],
			MIN_HEIGHT, MAX_HEIGHT, STEPS));
	}

	cod->lim = tile_count;
	for (i = 0; i < CHUNK_SIZE * CHUNK_SIZE; ++i) {
		ac_pack(cod, sck->tiles[i]);
	}
}

void
unpack_ser_chunk(struct ac_decoder *dec, struct ser_chunk *sck)
{
	uint32_t i, v[CHUNK_SIZE * CHUNK_SIZE];

	unpack_point(dec, &sck->cp, MAX_COORD, 0, CHUNK_SIZE);

	dec->lim = STEPS;
	ac_unpack(dec, v, CHUNK_SIZE * CHUNK_SIZE);
	for (i = 0; i < CHUNK_SIZE * CHUNK_SIZE; ++i) {
		sck->heights[i] =
			unquantizef(v[i], MIN_HEIGHT, MAX_HEIGHT, STEPS);
	}

	dec->lim = tile_count;
	ac_unpack(dec, v, CHUNK_SIZE * CHUNK_SIZE);
	for (i = 0; i < CHUNK_SIZE * CHUNK_SIZE; ++i) {
		sck->tiles[i] = v[i];
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
