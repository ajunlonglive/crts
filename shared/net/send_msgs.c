#include "posix.h"

#include <assert.h>
#include <errno.h>
#include <string.h>

#include "shared/net/connection.h"
#include "shared/net/net_ctx.h"
#include "shared/net/pool.h"
#include "shared/net/send_msgs.h"
#include "shared/serialize/net.h"
#include "shared/types/hdarr.h"
#include "shared/util/log.h"

struct send_ctx {
	struct net_ctx *nx;
	uint8_t *buf;
	size_t buflen;
	cx_bits_t send_to;
	struct msg_hdr hdr;
};


static enum iteration_result
transmit(void *_ctx, void *_cx)
{
	struct connection *cx = _cx;
	struct send_ctx *ctx = _ctx;

	if (!((ctx->hdr.ack) || (ctx->send_to & cx->bit))) {
		return ir_cont;
	}

	if (sendto(ctx->nx->sock, ctx->buf, ctx->buflen, 0,
		&cx->addr.sa, socklen) == -1) {
		LOG_W("sendto failed: %s", strerror(errno));
	}

	return ir_cont;
}

static void
send_msg(void *_ctx, msg_ack_t send_to, msg_seq_t seq, enum msg_flags f,
	void *msg, uint16_t len)
{
	struct send_ctx *ctx = _ctx;
	ctx->hdr = (struct msg_hdr){ .seq = seq };

	memset(ctx->buf, 0, BUFSIZE);

	uint16_t hdrlen = pack_msg_hdr(&ctx->hdr, ctx->buf, BUFSIZE);

	memcpy(ctx->buf + hdrlen, msg, len);
	ctx->buflen = hdrlen + len;

	assert(ctx->buflen < BUFSIZE);

	ctx->send_to = send_to;

	hdarr_for_each(ctx->nx->cxs.cxs, ctx, transmit);
}

static enum iteration_result
send_and_reset_cx_ack(void *_ctx, void *_cx)
{
	struct connection *cx = _cx;
	struct send_ctx *ctx = _ctx;

	ctx->hdr.seq = 0;
	ctx->hdr.ack = true;
	uint16_t hdrlen = pack_msg_hdr(&ctx->hdr, ctx->buf, BUFSIZE);

	ctx->buflen = hdrlen +
		      pack_acks(cx->acks, ctx->buf + hdrlen, BUFSIZE - hdrlen);

	transmit(ctx, cx);

	ack_clear_all(cx->acks);

	return ir_cont;
}

void
send_msgs(struct net_ctx *nx)
{
	static uint8_t buf[BUFSIZE] = { 0 };
	struct send_ctx ctx = {
		.nx = nx,
		.buf = buf
	};

	if (nx->buf.msg.mt) {
		msgq_add(nx->send, &nx->buf.msg, nx->buf.dest, nx->buf.f);
		memset(&nx->buf.msg, 0, sizeof(struct message));
	}

	msgq_send_all(nx->send, &ctx, send_msg);
	hdarr_for_each(nx->cxs.cxs, &ctx, send_and_reset_cx_ack);

	msgq_compact(nx->send);
}
