#ifndef SHARED_SERIALIZE_CODER_H
#define SHARED_SERIALIZE_CODER_H

#include <stddef.h>
#include <stdint.h>

struct ac_coder {
	uint64_t lim;
	uint32_t ceil, floor, pending;

	uint8_t *buf;
	uint32_t bufi, buflen;
};

struct ac_decoder {
	uint64_t lim;
	uint32_t ceil, floor, val;

	const uint8_t *buf;
	uint32_t bufi, buflen, bufused;
};

void ac_pack_init(struct ac_coder *c, uint8_t *buf, size_t blen);
void ac_pack(struct ac_coder *c, uint32_t val);
void ac_pack_finish(struct ac_coder *c);
void ac_unpack_init(struct ac_decoder *c, const uint8_t *buf, size_t blen);
void ac_unpack(struct ac_decoder *c, uint32_t out[], size_t len);
size_t ac_coder_len(struct ac_coder *c);
size_t ac_decoder_len(struct ac_decoder *c);
#endif
