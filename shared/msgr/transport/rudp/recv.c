#include "posix.h"

#include "shared/msgr/transport/rudp/packet.h"
#include "shared/msgr/transport/rudp/recv.h"
#include "shared/msgr/transport/rudp/util.h"
#include "shared/util/log.h"

struct unpack_ctx {
	struct msg_sender *sender;
	struct msgr *msgr;
};

static void
unpack_cb(void *_ctx, enum message_type mt, void *msg)
{
	struct unpack_ctx *ctx = _ctx;
	ctx->msgr->handler(ctx->msgr, mt, msg, ctx->sender);
}

void
rudp_recv_cb(uint8_t *msg, uint32_t len,
	const struct sock_addr *sender_addr, void *_ctx)
{
	struct msgr *msgr = _ctx;
	struct msgr_transport_rudp_ctx *ctx = msgr->transport_ctx;
	struct rudp_cx *cx;
	struct unpack_ctx uctx = { .msgr = msgr };
	struct packet_hdr phdr = { 0 };

	++ctx->stats.packets_recvd;

	cx = cx_get(&ctx->pool, sender_addr);

	packet_read_hdr(msg, &phdr);

	/* L(log_misc, "%x: recieved %s", msgr->id, (char *[]){ "normal", "ack", "connect" }[phdr.type]); */

	switch (phdr.type) {
	case packet_type_normal:
		if (!cx) {
			break;
		}

		cx->connected = true;

		uctx.sender = &cx->sender;

		seq_buf_insert(&cx->sb_recvd, phdr.seq);

		uint32_t bufi = PACKET_HDR_LEN;

		while (bufi < len) {
			bufi += unpack_message(&msg[bufi], len - bufi, unpack_cb, &uctx);
		}

		break;
	case packet_type_ack:
		if (!cx) {
			break;
		}

		cx->connected = true;

		packet_read_acks_and_process(&ctx->msg_sk_send, &cx->sb_sent,
			&msg[PACKET_HDR_LEN], len - PACKET_HDR_LEN, cx->sender.addr);

		break;
	case packet_type_connect:
		if (cx) {
			L(log_misc, "got hello, but already have cx");
			cx->connected = true;
			break;
		}

		struct packet_hello ph = { 0 };
		packet_read_hello(&msg[PACKET_HDR_LEN], &ph);

		cx = cx_add(&ctx->pool, sender_addr, ph.id);
		cx->connected = true;

		msgr->handler(msgr, mt_connect, NULL, &cx->sender);

		break;
	default:
		assert(false);
		break;
	}
}

void
rudp_recv(struct msgr *msgr)
{
	struct msgr_transport_rudp_ctx *ctx = msgr->transport_ctx;
	/* NOTE: It is OK this is static since recv isnt used when we are using the
	 * dummy impl */
	static uint8_t buf[PACKET_MAX_LEN] = { 0 };

	ctx->si->recv(ctx->sock, buf, PACKET_MAX_LEN, msgr, rudp_recv_cb);

	cx_prune(&ctx->pool, 10);
}
