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
	static struct msg_sender self = { .flags = msf_first_message };
	self.id = msgr->id;

	struct msgr *msgr_dest = msgr->transport_ctx;
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

		msgr_dest->handler(msgr_dest, msg->mt, smsg, &self);
	}

	self.flags &= ~msf_first_message;
}

void
msgr_transport_init_basic(struct msgr *msgr, struct msgr *dest)
{
	msgr->send = msgr_transport_send_basic;
	msgr->recv = msgr_transport_recv_basic;
	msgr->queue = msgr_transport_queue_basic;
	msgr->transport_ctx = dest;
}
