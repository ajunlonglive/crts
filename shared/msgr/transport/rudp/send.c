#include "posix.h"

#include <assert.h>

#include "shared/msgr/transport/rudp/packet.h"
#include "shared/msgr/transport/rudp/queue.h"
#include "shared/msgr/transport/rudp/send.h"
#include "shared/msgr/transport/rudp/util.h"
#include "shared/util/log.h"

static void
send_and_clear_packet(struct build_packet_ctx *bpc)
{
	struct msgr_transport_rudp_ctx *ctx = bpc->msgr->transport_ctx;

	++ctx->stats.packets_sent;
	if (bpc->bufi > ctx->stats.packet_size_max) {
		ctx->stats.packet_size_max = bpc->bufi;
	}

	/* L("_/"); */
	/* L("sending packet len:%d -> %d", bpc->bufi, bpc->cx->sock_addr.addr); */
	ctx->si->send(ctx->sock, (uint8_t *)bpc->buf, bpc->bufi, &bpc->cx->sock_addr);

	bpc->bufi = 0;
	++bpc->sent_packets;
}

static enum del_iter_result
build_packet_cb(void *_ctx, void *_hdr, void *itm, uint16_t len)
{
	struct build_packet_ctx *bpc = _ctx;
	struct msg_sack_hdr *hdr = _hdr;

	if (!(hdr->dest & bpc->cx->sender.addr)) {
		return dir_cont;
	} else if (hdr->send_cooldown) {
		--hdr->send_cooldown;
		return dir_cont;
	}
	hdr->send_cooldown = 10;

	if (bpc->bufi && !packet_space_available(bpc, len)) {
		send_and_clear_packet(bpc);
	}

	if (!bpc->bufi) {
		packet_write_setup(bpc, bpc->cx->local_seq, packet_type_normal, 0);
		++bpc->cx->local_seq;
		assert(packet_space_available(bpc, len));
	}

	/* L("  msg %d | times_sent: %d", hdr->msg_id, hdr->times_sent); */
	struct msgr_transport_rudp_ctx *ctx = bpc->msgr->transport_ctx;
	if (++hdr->times_sent > ctx->stats.msg_resent_max) {
		ctx->stats.msg_resent_max = hdr->times_sent;
	}

	switch (hdr->priority) {
	case priority_normal:
		packet_write_msg(bpc, hdr->msg_id, itm, len, true);

		return dir_cont;
	case priority_dont_resend:
		packet_write_msg(bpc, hdr->msg_id, itm, len, false);

		return dir_del;
	default:
		assert(false);
		return dir_cont;
	}
}

void
send_connect(struct msgr *msgr, struct build_packet_ctx *bpc)
{
	packet_write_setup(bpc, 0, packet_type_connect, 0);
	packet_write_hello(bpc, msgr->id);
	send_and_clear_packet(bpc);
}

void
send_acks(struct msgr *msgr, struct build_packet_ctx *bpc)
{
	packet_write_setup(bpc, 0, packet_type_ack, 0);
	packet_write_acks(bpc);
	send_and_clear_packet(bpc);
}

void
rudp_send(struct msgr *msgr)
{
	struct msgr_transport_rudp_ctx *ctx = msgr->transport_ctx;
	struct build_packet_ctx bpc = { .msgr = msgr };

	uint32_t i;
	for (i = 0; i < hdarr_len(&ctx->pool.cxs); ++i) {
		bpc.cx = hdarr_get_by_i(&ctx->pool.cxs, i);
		bpc.sent_msgs = bpc.sent_packets = 0; // TODO: debug only?
		/* L("%x: sending to cx:%x", msgr->id, bpc.cx->id); */

		if (!bpc.cx->connected) {
			L("sending hello");
			send_connect(msgr, &bpc);
			continue;
		}

		sack_iter(&ctx->msg_sk_send, &bpc, build_packet_cb);
		if (bpc.bufi) {
			send_and_clear_packet(&bpc);
		}

		if (bpc.cx->sb_recvd.last_acked != bpc.cx->sb_recvd.head) {
			send_acks(msgr, &bpc);

			bpc.cx->sb_recvd.last_acked = bpc.cx->sb_recvd.head;
		}

		/* L("sack: %d/%dkb | sent packets: %d sent msgs: %d, resent max: %d", ctx->msg_sk_send.items, */
		/* 	ctx->msg_sk_send.len / 1000, bpc.sent_packets, bpc.sent_msgs, ctx->stats.msg_resent_max); */
	}
}
