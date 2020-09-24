#include "posix.h"

#include <string.h>

#include "shared/net/connection.h"
#include "shared/net/pool.h"
#include "shared/net/recv_msgs.h"
#include "shared/serialize/message.h"
#include "shared/serialize/net.h"
#include "shared/types/darr.h"
#include "shared/util/log.h"

struct unpack_msg_ctx {
	struct net_ctx *nx;
	struct connection *cx;
};

static void
unpack_msg_cb(void *_ctx, enum message_type mt, void *msg)
{
	struct unpack_msg_ctx *ctx = _ctx;
	ctx->nx->handler(ctx->nx, mt, msg, ctx->cx);
}

void
recv_msgs(struct net_ctx *ctx)
{
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
		if ((cx = cx_establish(&ctx->cxs, &caddr.ia)) == NULL) {
			continue;
		}


		size_t hdrlen = unpack_msg_hdr(&mh, buf, blen);

		if (mh.ack) {
			/* TODO ack */
			unpack_acks(acks, buf + hdrlen, blen - hdrlen);

			ack_msgq(acks, ctx->send, cx->bit);
			continue;
		}

		ack_set(cx->acks, mh.seq);

		struct unpack_msg_ctx uctx = { ctx, cx, };

		assert(hdrlen < (uint16_t)blen);

		unpack_message(buf + hdrlen, blen - hdrlen, unpack_msg_cb, &uctx);
	}

	cx_prune(&ctx->cxs, 10);
}
