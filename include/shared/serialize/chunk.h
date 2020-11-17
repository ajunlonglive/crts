#ifndef SHARED_SERIALIZE_CHUNK_H
#define SHARED_SERIALIZE_CHUNK_H

#include "shared/serialize/coder.h"
#include "shared/sim/chunk.h"

struct ser_chunk {
	struct point cp;
	uint8_t tiles[CHUNK_SIZE * CHUNK_SIZE];
	float heights[CHUNK_SIZE * CHUNK_SIZE];
};

void fill_ser_chunk(struct ser_chunk *sck, const struct chunk *ck);
void unfill_ser_chunk(const struct ser_chunk *sck, struct chunk *ck);
void pack_ser_chunk(struct ac_coder *cod, const struct ser_chunk *sck);
void unpack_ser_chunk(struct ac_decoder *dec, struct ser_chunk *sck);
size_t unpack_chunk(struct chunk *ck, const uint8_t *buf, size_t blen);
size_t pack_chunk(const struct chunk *ck, uint8_t *buf, size_t blen);
#endif
