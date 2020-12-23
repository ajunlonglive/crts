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
#include "tracy.h"
#include "version.h"

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
	TracyCZoneAutoS;
	struct connection *cx = _cx;
	struct send_ctx *ctx = _ctx;

	if (!(ctx->send_to & cx->bit)) {
		goto return_continue;
	}

	if (sendto(ctx->nx->sock, ctx->buf, ctx->buflen, 0,
		&cx->addr.sa, socklen) == -1) {
		LOG_W("sendto failed: %s", strerror(errno));
	}

return_continue:
	TracyCZoneAutoE;
	return ir_cont;
}

static void
send_msg(void *_ctx, msg_ack_t send_to, msg_seq_t seq, enum msg_flags f,
	void *msg, uint16_t len)
{
	TracyCZoneAutoS;
	struct send_ctx *ctx = _ctx;
	ctx->hdr = (struct msg_hdr){ .seq = seq, .kind = mk_msg };

	memset(ctx->buf, 0, BUFSIZE);

	uint16_t hdrlen = pack_msg_hdr(&ctx->hdr, ctx->buf, BUFSIZE);

	memcpy(ctx->buf + hdrlen, msg, len);
	ctx->buflen = hdrlen + len;

	assert(ctx->buflen < BUFSIZE);

	ctx->send_to = send_to;

	hdarr_for_each(&ctx->nx->cxs.cxs, ctx, transmit);
	TracyCZoneAutoE;
}

static enum iteration_result
send_hello_if_new(void *_ctx, void *_cx)
{
	struct connection *cx = _cx;
	struct send_ctx *ctx = _ctx;

	if (!cx->new) {
		return ir_cont;
	}

	memset(ctx->buf, 0, BUFSIZE);

	ctx->hdr.seq = 0;
	ctx->hdr.kind = mk_hello;
	uint16_t hdrlen = pack_msg_hdr(&ctx->hdr, ctx->buf, BUFSIZE);

	struct msg_hello hello = { .id = ctx->nx->id };
	uint16_t len = strlen(crts_version.version);
	assert(len < VERSION_LEN);
	memcpy(&hello.version, crts_version.version, len);


	ctx->buflen = hdrlen + pack_hello(&hello, ctx->buf + hdrlen, BUFSIZE - hdrlen);

	ctx->send_to = cx->bit;
	transmit(ctx, cx);

	cx->new = false;

	return ir_cont;
}

static enum iteration_result
send_and_reset_cx_ack(void *_ctx, void *_cx)
{
	TracyCZoneAutoS;
	struct connection *cx = _cx;
	struct send_ctx *ctx = _ctx;
	memset(ctx->buf, 0, BUFSIZE);

	ctx->hdr.seq = 0;
	ctx->hdr.kind = mk_ack;
	uint16_t hdrlen = pack_msg_hdr(&ctx->hdr, ctx->buf, BUFSIZE);

	ctx->buflen = hdrlen + pack_acks(&cx->acks, ctx->buf + hdrlen, BUFSIZE - hdrlen);

	ctx->send_to = cx->bit;
	transmit(ctx, cx);

	ack_clear_all(&cx->acks);

	TracyCZoneAutoE;
	return ir_cont;
}

void
send_msgs(struct net_ctx *nx)
{
	TracyCZoneAutoS;
	static uint8_t buf[BUFSIZE] = { 0 };
	struct send_ctx ctx = {
		.nx = nx,
		.buf = buf
	};

	if (nx->buf.msg.mt) {
		msgq_add(&nx->send, &nx->buf.msg, nx->buf.dest, nx->buf.f);
		memset(&nx->buf.msg, 0, sizeof(struct message));
	}

	hdarr_for_each(&nx->cxs.cxs, &ctx, send_hello_if_new);
	msgq_send_all(&nx->send, &ctx, send_msg);
	hdarr_for_each(&nx->cxs.cxs, &ctx, send_and_reset_cx_ack);

	msgq_compact(&nx->send);
	TracyCZoneAutoE;
}
