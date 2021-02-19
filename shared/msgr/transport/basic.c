#include "posix.h"

#include "shared/msgr/transport/basic.h"
#include "shared/util/log.h"

static void
msgr_transport_send_basic(struct msgr *msgr)
{
	/* NOOP */
}

static void
msgr_transport_recv_basic(struct msgr *msgr)
{
	/* NOOP */
}

static void
msgr_transport_queue_basic(struct msgr *msgr, struct message *msg, msg_addr_t dest)
{
	struct msgr_transport_basic_ctx *ctx = msgr->transport_ctx;
	void *smsg;

	uint32_t i;
	for (i = 0; i < msg->count; ++i) {
		switch (msg->mt) {
		case mt_poke:
			break;
		case mt_req:
			smsg = &msg->dat.req[i];
			break;
		case mt_ent:
			smsg = &msg->dat.ent[i];
			break;
		case mt_action:
			smsg = &msg->dat.action[i];
			break;
		case mt_tile:
			smsg = &msg->dat.tile[i];
			break;
		case mt_chunk:
			smsg = &msg->dat.chunk[i];
			break;
		default:
			assert(false);
		}

		ctx->msgr_dest->handler(ctx->msgr_dest, msg->mt, smsg, &ctx->self);
		ctx->self.flags &= ~msf_first_message;
	}
}

void
msgr_transport_init_basic(struct msgr *msgr, struct msgr *dest,
	struct msgr_transport_basic_ctx *ctx)
{
	msgr->transport_impl = msgr_transport_basic;

	*ctx = (struct msgr_transport_basic_ctx) {
		.self = {
			.flags = msf_first_message,
			.id = msgr->id
		},
		.msgr_dest = dest,
	};

	msgr->send = msgr_transport_send_basic;
	msgr->recv = msgr_transport_recv_basic;
	msgr->queue = msgr_transport_queue_basic;
	msgr->transport_ctx = ctx;
}
