#include "posix.h"

#include "shared/msgr/transport/basic.h"
#include "shared/util/log.h"

struct msgr_transport_basic_ctx {
	struct msg_sender sender;
	struct ring_buffer *in, *out;
	bool sent_first_msg;
};

static void
msgr_transport_send_basic(struct msgr *msgr)
{
	/* NOOP */
}

static void
msgr_transport_recv_basic(struct msgr *msgr)
{
	struct msgr_transport_basic_ctx *ctx = msgr->transport_ctx;
	struct message *msg;
	void *smsg;

	while ((msg = ring_buffer_pop(ctx->in))) {
		if (!ctx->sent_first_msg ) {
			msgr->handler(msgr, mt_connect, NULL, &ctx->sender);
			ctx->sent_first_msg = true;
		}

		uint32_t i;
		for (i = 0; i < msg->count; ++i) {
			switch (msg->mt) {
			case mt_poke:
				smsg = NULL;
				break;
			case mt_req:
				smsg = &msg->dat.req[i];
				break;
			case mt_ent:
				smsg = &msg->dat.ent[i];
				break;
			case mt_tile:
				smsg = &msg->dat.tile[i];
				break;
			case mt_chunk:
				smsg = &msg->dat.chunk[i];
				break;
			case mt_cursor:
				smsg = &msg->dat.cursor[i];
				break;
			case mt_server_info:
				smsg = &msg->dat.server_info[i];
				break;
			case mt_server_cmd:
				smsg = &msg->dat.server_cmd[i];
				break;
			case mt_connect:
			case message_type_count:
				assert(false);
				smsg = NULL;
				continue;
			}

			msgr->handler(msgr, msg->mt, smsg, &ctx->sender);
		}
	}
}

static void
msgr_transport_queue_basic(struct msgr *msgr, struct message *msg, msg_addr_t dest,
	enum msg_priority_type _priority)
{
	struct msgr_transport_basic_ctx *ctx = msgr->transport_ctx;
	bool succ;

	succ = ring_buffer_push(ctx->out, msg);

	if (!succ) {
		LOG_W(log_net, "dropping message");
	}
}

static void
init_basic(struct msgr *msgr, uint32_t sender_id, struct ring_buffer *in,
	struct ring_buffer *out, struct msgr_transport_basic_ctx *ctx)
{
	msgr->transport_impl = msgr_transport_basic;

	*ctx = (struct msgr_transport_basic_ctx) {
		.sender = { .id = sender_id },
		.in = in,
		.out = out,
	};

	msgr->send = msgr_transport_send_basic;
	msgr->recv = msgr_transport_recv_basic;
	msgr->queue = msgr_transport_queue_basic;
	msgr->transport_ctx = ctx;
}

void
msgr_transport_init_basic_pipe(struct msgr *a, struct msgr *b)
{
	static bool called = false;
	assert(!called && "msgr_transport_init_basic_pipe called twice");
	called = true;

	static struct ring_buffer rbuf[2] = { 0 };

	ring_buffer_init(&rbuf[0], sizeof(struct message), 1024);
	ring_buffer_init(&rbuf[1], sizeof(struct message), 1024);

	static struct msgr_transport_basic_ctx transport_basic_ctx[2] = { 0 };

	init_basic(a, b->id, &rbuf[0], &rbuf[1], &transport_basic_ctx[0]);
	init_basic(b, a->id, &rbuf[1], &rbuf[0], &transport_basic_ctx[1]);
}
