#include <err.h>

#include "shared/net/connection.h"
#include "shared/net/pool.h"
#include "shared/net/send_msgs.h"
#include "shared/serialize/net.h"
#include "shared/types/hdarr.h"
#include "shared/util/log.h"

struct send_ctx {
	const struct send_msgs_ctx *mctx;
	struct msg_hdr hdr;
	msg_ack_t send_to;
	char *buf;
	size_t buflen;
};

static enum iteration_result
transmit(void *_ctx, void *_cx)
{
	struct connection *cx = _cx;
	struct send_ctx *ctx = _ctx;

	if (!((ctx->hdr.flags & msgf_ack) || (ctx->send_to & cx->bit))) {
		return ir_cont;
	}

	if (sendto(ctx->mctx->sock, ctx->buf, ctx->buflen, 0, &cx->addr.sa, socklen) == -1) {
		warn("sendto failed");
	}

	return ir_cont;
}

static void
send_msg(void *_ctx, msg_ack_t send_to, msg_seq_t seq, enum msg_flags f, void *msg)
{
	struct send_ctx *ctx = _ctx;

	ctx->hdr.seq = seq;
	ctx->hdr.flags = f;
	pack_msg_hdr(&ctx->hdr, ctx->buf);

	ctx->buflen = MSG_HDR_LEN + ctx->mctx->packer(msg, ctx->buf + MSG_HDR_LEN);

	ctx->send_to = send_to;

	hdarr_for_each(ctx->mctx->cxs->cxs, ctx, transmit);
}

static enum iteration_result
send_and_reset_cx_ack(void *_ctx, void *_cx)
{
	struct connection *cx = _cx;
	struct send_ctx *ctx = _ctx;

	ctx->hdr.seq = 0;
	ctx->hdr.flags = msgf_ack;
	pack_msg_hdr(&ctx->hdr, ctx->buf);

	ctx->buflen = MSG_HDR_LEN + pack_acks(&cx->acks, &ctx->buf[MSG_HDR_LEN]);

	transmit(ctx, cx);

	ack_clear_all(&cx->acks);

	return ir_cont;
}

void
send_msgs(const struct send_msgs_ctx *mctx)
{
	static char buf[BUFSIZE] = { 0 };
	struct send_ctx ctx = {
		.mctx = mctx,
		.buf = buf
	};

	msgq_send_all(mctx->send, &ctx, send_msg);
	hdarr_for_each(mctx->cxs->cxs, &ctx, send_and_reset_cx_ack);
}
