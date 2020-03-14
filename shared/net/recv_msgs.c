#include <string.h>

#include "shared/net/pool.h"
#include "shared/net/recv_msgs.h"
#include "shared/serialize/net.h"
#include "shared/types/darr.h"
#include "shared/util/log.h"

void
recv_msgs(const struct recv_msgs_ctx *ctx)
{
	static char buf[BUFSIZE];
	int blen;
	struct connection *cx;
	struct msg_hdr mh;
	struct acks acks;
	void *mem;

	union {
		struct sockaddr_in ia;
		struct sockaddr sa;
	} caddr;

	while ((blen = recvfrom(ctx->sock, buf, BUFSIZE, 0, &caddr.sa, &socklen)) > 0) {
		if ((cx = cx_establish(ctx->cxs, &caddr.ia)) == NULL) {
			continue;
		}

		unpack_msg_hdr(&mh, buf);

		if (mh.flags & msgf_ack) {
			unpack_acks(&acks, buf + MSG_HDR_LEN);
			msgq_ack(ctx->sent, &acks, cx->bit);
			continue;
		}

		ack_set(&cx->acks, mh.seq);

		mem = darr_get_mem(ctx->out);
		ctx->unpacker(mem, cx, buf + MSG_HDR_LEN);
	}

	msgq_flush(ctx->sent);

	cx_prune(ctx->cxs, 10);
}
