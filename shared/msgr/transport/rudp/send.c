#include "posix.h"

#include <assert.h>

#include "shared/msgr/transport/rudp/packet.h"
#include "shared/msgr/transport/rudp/queue.h"
#include "shared/msgr/transport/rudp/send.h"
#include "shared/util/log.h"

#if 0
static enum del_iter_result
send_cb(void *_ctx, void *_hdr, void *itm, uint16_t len)
{
	struct msgr *msgr = _ctx;
	struct msgr_transport_rudp_ctx *ctx = msgr->transport_ctx;
	struct msg_sack_hdr *hdr = _hdr;
	struct rudp_cx *cx;
	uint32_t ack_bits;
	uint16_t n;
	char buf[MTU] = { 0 };

	if (hdr->send_cooldown) {
		--hdr->send_cooldown;
		return dir_cont;
	}
	hdr->send_cooldown = 2;

	assert(len < MTU - HDR_SIZE);
	memcpy(&buf[HDR_SIZE], itm, len);

	uint32_t i;
	for (i = 0; i < hdarr_len(&ctx->pool.cxs); ++i) {
		cx = hdarr_get_by_i(&ctx->pool.cxs, i);

		if (!(hdr->dest & cx->addr)) {
			continue;
		}

		/* L("sending: seq:%d, id:%x, ack:%d, ack_bits:%s", hdr->seq, msgr->id, cx->remote_seq, */
		/* 	ack_bits_to_s(gen_ack_bits(cx))); */

		/* unpack_message(itm, len, tmp_inspect_unpack_cb, NULL); */
		ctx->si->send(ctx->sock, (uint8_t *)buf, len + HDR_SIZE, &cx->sock_addr);
	}

	return dir_cont;
}
#endif

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
}

static enum del_iter_result
build_packet_cb(void *_ctx, void *_hdr, void *itm, uint16_t len)
{
	struct build_packet_ctx *bpc = _ctx;
	struct msg_sack_hdr *hdr = _hdr;

	if (!(hdr->dest & bpc->cx->addr)) {
		return dir_cont;
	}

	if (bpc->bufi && !packet_space_available(bpc, len)) {
		send_and_clear_packet(bpc);
	}

	if (!bpc->bufi) {
		packet_write_setup(bpc, bpc->cx->local_seq, bpc->msgr->id);
		++bpc->cx->local_seq;
		assert(packet_space_available(bpc, len));
	}

	/* L("  msg %d | times_sent: %d", hdr->msg_id, hdr->times_sent); */
	struct msgr_transport_rudp_ctx *ctx = bpc->msgr->transport_ctx;
	if (++hdr->times_sent > ctx->stats.msg_resent_max) {
		ctx->stats.msg_resent_max = hdr->times_sent;
	}

	packet_write_msg(bpc, hdr->msg_id, itm, len);

	return dir_cont;
}

void
rudp_send(struct msgr *msgr)
{
	struct msgr_transport_rudp_ctx *ctx = msgr->transport_ctx;
	struct build_packet_ctx bpc = { .msgr = msgr };

	if (!ctx->msg_sk_send.items) {
		rudp_queue(msgr, &(struct message){ .mt = mt_poke }, 0);
	}

	uint32_t i;
	for (i = 0; i < hdarr_len(&ctx->pool.cxs); ++i) {
		bpc.cx = hdarr_get_by_i(&ctx->pool.cxs, i);
		/* L("sending to cx:%x", bpc.cx->id); */

		sack_iter(&ctx->msg_sk_send, &bpc, build_packet_cb);

		if (bpc.bufi) {
			send_and_clear_packet(&bpc);
		}
	}
}
