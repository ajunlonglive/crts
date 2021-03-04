#include "posix.h"

#include "shared/msgr/transport/rudp/queue.h"
#include "shared/util/log.h"

/* queue */

void
rudp_queue(struct msgr *msgr, struct message *msg, msg_addr_t dest)
{
	struct msgr_transport_rudp_ctx *ctx = msgr->transport_ctx;

	if (!dest) {
		if (!ctx->pool.used_addrs) {
			return;
		}

		dest = ctx->pool.used_addrs;
	}

	struct msg_sack_hdr hdr = { .dest = dest, .msg_id = ctx->msg_id };
	++ctx->msg_id;

	/* L("sending ~ %d:%s", hdr.msg_id, inspect_message(msg->mt, msg)); */

	sack_stuff(&ctx->msg_sk_send, &hdr, msg);
}
