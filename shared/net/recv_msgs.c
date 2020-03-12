#include "shared/net/pool.h"
#include "shared/net/recv_msgs.h"
#include "shared/serialize/misc.h"
#include "shared/types/darr.h"
#include "shared/util/log.h"

void
recv_msgs(const struct recv_msgs_ctx *ctx)
{
	static char buf[BUFSIZE];
	int blen;
	struct connection *cx;
	struct msg_hdr mh;
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

		/*
		   if (mh.msg_seq > cx->ack_seq) {
		        cx->ack <<= (mh.msg_seq - cx->ack_seq);
		        cx->ack_seq = mh.msg_seq;
		        cx->ack |= 1;
		   } else if (cx->ack_seq - mh.msg_seq < FRAMELEN) {
		        cx->ack |= 1 << (mh.msg_seq - cx->ack_seq);
		   } else {
		        L("unable to ack message");
		   }

		   msgq_ack(ctx->sent, mh.ack_seq, mh.ack, cx->bit);
		 */

		mem = darr_get_mem(ctx->out);
		ctx->unpacker(mem, cx, buf + MSG_HDR_LEN);
	}

	msgq_flush(ctx->sent);

	cx_prune(ctx->cxs, 10);
}
