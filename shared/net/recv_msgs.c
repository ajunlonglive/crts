#include "posix.h"

#include <string.h>

#include "shared/net/connection.h"
#include "shared/net/pool.h"
#include "shared/net/recv_msgs.h"
#include "shared/serialize/message.h"
#include "shared/serialize/net.h"
#include "shared/types/darr.h"
#include "shared/util/log.h"
#include "tracy.h"
#include "version.h"

struct unpack_msg_ctx {
	struct net_ctx *nx;
	struct connection *cx;
};

static void
unpack_msg_cb(void *_ctx, enum message_type mt, void *msg)
{
	TracyCZoneAutoS;
	struct unpack_msg_ctx *ctx = _ctx;
	ctx->nx->handler(ctx->nx, mt, msg, ctx->cx);
	TracyCZoneAutoE;
}

void
recv_msgs(struct net_ctx *ctx)
{
	TracyCZoneAutoS;

	static uint8_t buf[BUFSIZE] = { 0 };
	int blen;
	struct connection *cx;
	struct msg_hdr mh;
	static struct hash *acks = NULL;
	if (!acks) {
		acks = ack_init();
	}

	union {
		struct sockaddr_in ia;
		struct sockaddr sa;
	} caddr;

	while ((blen = recvfrom(ctx->sock, buf, BUFSIZE, 0, &caddr.sa, &socklen)) > 0) {
		TracyCZoneN(tctx_process_message, "process message", true);
		cx = cx_establish(&ctx->cxs, &caddr.ia);

		size_t hdrlen = unpack_msg_hdr(&mh, buf, blen);

		switch (mh.kind) {
		case mk_hello:
			if (cx) {
				cx->new = true;
				goto done_processing;
			}

			struct msg_hello hello = { 0 };
			unpack_hello(&hello, buf + hdrlen, blen - hdrlen);

			if (strcmp(VERSION, (char *)hello.version) != 0) {
				LOG_W("connecton attemped with bad version: %s != %s",
					VERSION, hello.version);
				goto done_processing;
			}

			cx_add(&ctx->cxs, &caddr.ia, hello.id);

			break;
		case mk_ack:
			if (!cx) {
				goto done_processing;
			}

			/* TODO ack */
			unpack_acks(acks, buf + hdrlen, blen - hdrlen);

			ack_msgq(acks, ctx->send, cx->bit);
			break;
		case mk_msg:
			if (!cx) {
				goto done_processing;
			}

			ack_set(cx->acks, mh.seq);

			struct unpack_msg_ctx uctx = { ctx, cx, };

			assert(hdrlen < (uint16_t)blen);

			unpack_message(buf + hdrlen, blen - hdrlen, unpack_msg_cb, &uctx);
			break;
		default:
			assert(false);
		}

done_processing:
		TracyCZoneEnd(tctx_process_message);
	}

	cx_prune(&ctx->cxs, 10);

	TracyCZoneAutoE;
}
