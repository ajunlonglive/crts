#ifndef SHARED_SERIALIZE_CODER_H
#define SHARED_SERIALIZE_CODER_H

#include <stddef.h>
#include <stdint.h>

struct ac_coder {
	uint32_t lim;
	uint32_t ceil, floor, pending;

	uint8_t *buf;
	uint32_t bufi, buflen;
};

struct ac_decoder {
	uint32_t lim;
	uint32_t ceil, floor, val;

	const uint8_t *buf;
	uint32_t bufi, buflen;
};

void ac_pack_init(struct ac_coder *c);
void ac_pack(struct ac_coder *c, uint32_t val);
void ac_pack_finish(struct ac_coder *c);
void ac_unpack_init(struct ac_decoder *c);
void ac_unpack(struct ac_decoder *c, uint32_t out[], size_t len);
#endif
