#include "posix.h"

#include "shared/msgr/transport/rudp/packet.h"
#include "shared/msgr/transport/rudp/recv.h"
#include "shared/msgr/transport/rudp/util.h"
#include "shared/util/log.h"

struct unpack_ctx {
	struct msg_sender sender;
	struct msgr *msgr;
};

static void
unpack_cb(void *_ctx, enum message_type mt, void *msg)
{
	struct unpack_ctx *ctx = _ctx;
	ctx->msgr->handler(ctx->msgr, mt, msg, &ctx->sender);
	ctx->sender.flags &= ~msf_first_message;
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

	packet_read_hdr(msg, &phdr);
	uctx.sender.id = phdr.sender_id;

	if (!(cx = cx_get(&ctx->pool, sender_addr))) {
		cx = cx_add(&ctx->pool, sender_addr, phdr.sender_id);
		uctx.sender.flags |= msf_first_message;
	}

	uctx.sender.addr = cx->addr;

	/* L("recvd: seq: %d, id: %x, ack: %d, ack_bits: %s", phdr.seq, phdr.sender_id, phdr.ack, */
	/* 	ack_bits_to_s(phdr.ack_bits)); */

	packet_ack_process(&ctx->msg_sk_send, &cx->sb_sent, phdr.ack,
		phdr.ack_bits, cx->addr);

	seq_buf_insert(&cx->sb_recvd, phdr.seq);

	/* L("id: %x, seq: %d", phdr.id, phdr.seq); */

	/* unpack_packet_hdr(msg, blen - hdrlen, unpack_cb, &uctx); */

	uint32_t bufi = PACKET_HDR_LEN;

	while (bufi < len) {
		bufi += unpack_message(&msg[bufi], len - bufi, unpack_cb, &uctx);
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
