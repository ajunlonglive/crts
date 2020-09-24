#include "posix.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "shared/serialize/coder.h"
#include "shared/serialize/net.h"
#include "shared/util/log.h"

size_t
pack_msg_hdr(const struct msg_hdr *mh, uint8_t *buf, uint32_t blen)
{
	struct ac_coder cod;
	ac_pack_init(&cod, buf, blen);

	cod.lim = 2;
	ac_pack(&cod, mh->ack);

	cod.lim = UINT16_MAX;
	ac_pack(&cod, mh->seq);

	ac_pack_finish(&cod);

	if (ac_coder_len(&cod) != 3) {
		L("%d", cod.bufi);
		log_bytes(cod.buf, 4);
		printf("\n");
	}
	assert(ac_coder_len(&cod) == 3);
	return ac_coder_len(&cod);
}

size_t
unpack_msg_hdr(struct msg_hdr *mh, const uint8_t *buf, uint32_t blen)
{
	struct ac_decoder dec;
	uint32_t v;

	ac_unpack_init(&dec, buf, blen);

	dec.lim = 2;
	ac_unpack(&dec, &v, 1);
	mh->ack = v == 1;

	dec.lim = UINT16_MAX;
	ac_unpack(&dec, &v, 1);
	mh->seq = v;

	if (ac_decoder_len(&dec) != 3) {
		L("%d", dec.bufi);
		log_bytes(dec.buf, 4);
		printf("\n");
	}
	assert(ac_decoder_len(&dec) == 3);
	return ac_decoder_len(&dec);
}

struct pack_acks_ctx {
	uint8_t *buf;
	uint32_t blen, bufi;
};

static enum iteration_result
pack_acks_iter(void *_ctx, void *_key, size_t _ack)
{
	struct pack_acks_ctx *ctx = _ctx;
	msg_seq_t key = *(msg_seq_t *)_key;
	uint32_t ack = _ack;

	uint16_t nk = htons(key);
	uint32_t nv = htonl(ack);

	memcpy(&ctx->buf[ctx->bufi], &nk, sizeof(uint16_t));
	ctx->bufi += sizeof(uint16_t);

	assert(ctx->bufi < ctx->blen);

	memcpy(&ctx->buf[ctx->bufi], &nv, sizeof(uint32_t));
	ctx->bufi += sizeof(uint32_t);

	assert(ctx->bufi < ctx->blen);

	return ir_cont;
}

size_t
pack_acks(struct hash *a, uint8_t *buf, uint32_t blen)
{
	struct pack_acks_ctx ctx = { .buf = buf, .blen = blen };
	hash_for_each_with_keys(a, &ctx, pack_acks_iter);
	return ctx.bufi;
}

size_t
unpack_acks(struct hash *a, const uint8_t *buf, uint32_t blen)
{
	hash_clear(a);

	uint32_t i;
	for (i = 0; i < blen;) {
		msg_seq_t nk = ntohs(*(uint16_t *)&buf[i]);
		i += sizeof(uint16_t);
		uint32_t hv = ntohl(*(uint32_t *)&buf[i]);
		i += sizeof(uint32_t);

		hash_set(a, &nk, hv);
	}

	return i;
}
