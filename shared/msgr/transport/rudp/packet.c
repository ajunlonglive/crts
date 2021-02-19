#include "posix.h"

#include <assert.h>
#include <string.h>

#include "shared/msgr/transport/rudp/packet.h"
#include "shared/msgr/transport/rudp/seq_buf.h"
#include "shared/msgr/transport/rudp/util.h"
#include "shared/serialize/byte_swappers.h"
#include "shared/util/log.h"

#define PACKET_MAX_MSGS 32

struct sb_elem_packet {
	uint16_t msg[PACKET_MAX_MSGS];
	uint8_t msgs;
	bool acked;
};

const char *
ack_bits_to_s(uint32_t ack_bits)
{
	uint32_t i;
	static char buf[64] = { 0 };

	for (i = 0; i < 32; ++i) {
		buf[i] = (ack_bits & (1 << i)) ? '1' : '0';
	}

	buf[i] = 0;

	return buf;
}

struct ack_msgs_cb_ctx {
	uint16_t *msg, msgs;
	msg_addr_t acker;
};

static enum del_iter_result
ack_msgs_cb(void *_ctx, void *_hdr, void *itm, uint16_t len)
{
	struct ack_msgs_cb_ctx *ctx  = _ctx;
	struct msg_sack_hdr *hdr = _hdr;

	uint32_t i;
	for (i = 0; i < ctx->msgs; ++i) {
		if (hdr->msg_id == ctx->msg[i]) {
			/* L("--> acked message %d", ctx->msg[i]); */
			if (i < ctx->msgs - 1u) {
				ctx->msg[i] = ctx->msg[ctx->msgs - 1u];
			}
			--ctx->msgs;

			hdr->dest &= ~ctx->acker;
			if (!hdr->dest) {
				return dir_del;
			}
		}
	}

	if (ctx->msgs) {
		return dir_cont;
	} else {
		return dir_break;
	}
}

void
packet_ack_process(struct sack *sk, struct seq_buf *sent, uint16_t ack,
	uint32_t ack_bits, msg_addr_t acker)
{
	uint16_t i;
	struct sb_elem_packet *ep;

	/* L("--> processing ack %d:%s", ack, ack_bits_to_s(ack_bits)); */

	for (i = 0; i < 32; ++i) {
		if (!(ack_bits & (1 << i))) {
			continue;
		} else if (!(ep = seq_buf_get(sent, ack - i))) {
			continue;
		} else if (ep->acked) {
			continue;
		}

		ep->acked = true;

		struct ack_msgs_cb_ctx ctx = {
			.msg = ep->msg, .msgs = ep->msgs,
			.acker = acker,
		};

		sack_iter(sk, &ctx, ack_msgs_cb);
	}
}

static void
packet_write(struct build_packet_ctx *bpc, void *itm, uint16_t len)
{
	memcpy(&bpc->buf[bpc->bufi], itm, len);
	bpc->bufi += len;
}

bool
packet_space_available(struct build_packet_ctx *bpc, uint16_t len)
{
	struct sb_elem_packet *ep;

	return bpc->bufi + len < PACKET_MAX_LEN
	       && (ep = seq_buf_get(&bpc->cx->sb_sent, bpc->seq))
	       && ep->msgs + 1 < PACKET_MAX_MSGS;
}

void
packet_write_msg(struct build_packet_ctx *bpc, uint16_t id,
	void *itm, uint16_t len)
{
	struct msgr_transport_rudp_ctx *ctx = bpc->msgr->transport_ctx;
	struct sb_elem_packet *ep = seq_buf_get(&bpc->cx->sb_sent, bpc->seq);
	assert(ep);

	ep->msg[ep->msgs] = id;
	++ep->msgs;

	if (ep->msgs > ctx->stats.packet_msg_count_max) {
		ctx->stats.packet_msg_count_max = ep->msgs;
	}
	++ctx->stats.messages_sent;

	packet_write(bpc, itm, len);
}

void
packet_write_setup(struct build_packet_ctx *bpc, uint16_t seq, uint16_t id)
{
	uint16_t n;
	uint32_t nl;

	bpc->seq = seq;

	struct sb_elem_packet *ep = seq_buf_insert(&bpc->cx->sb_sent, bpc->seq);
	assert(ep);
	*ep = (struct sb_elem_packet) { 0 };

	n = host_to_net_16(seq);
	packet_write(bpc, &n, 2);

	n = host_to_net_16(id);
	packet_write(bpc, &n, 2);

	n = host_to_net_16(bpc->cx->sb_recvd.head);
	packet_write(bpc, &n, 2);

	/* L("/1* -%x- packet:%d\n  ack %d:%s", id, seq, */
	/* 	bpc->cx->sb_recvd.head, */
	/* 	ack_bits_to_s(seq_buf_gen_ack_bits(&bpc->cx->sb_recvd))); */

	nl = host_to_net_32(seq_buf_gen_ack_bits(&bpc->cx->sb_recvd));
	packet_write(bpc, &nl, 4);
}

void
packet_read_hdr(const uint8_t *msg, struct packet_hdr *phdr)
{
	memcpy(&phdr->seq,       &msg[0], 2);
	memcpy(&phdr->sender_id, &msg[2], 2);
	memcpy(&phdr->ack,       &msg[4], 2);
	memcpy(&phdr->ack_bits,  &msg[6], 4);

	phdr->seq       = net_to_host_16(phdr->seq);
	phdr->sender_id = net_to_host_16(phdr->sender_id);
	phdr->ack       = net_to_host_16(phdr->ack);
	phdr->ack_bits  = net_to_host_32(phdr->ack_bits);
}
