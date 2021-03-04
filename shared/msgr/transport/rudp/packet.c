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
packet_read_acks_and_process(struct sack *sk, struct seq_buf *sent,
	const uint8_t *msg, uint32_t len, msg_addr_t acker)
{
	assert(len >= 2 + 4);

	uint16_t i = 0, biti = 0;
	struct sb_elem_packet *ep;
	const uint16_t ack = net_to_host_16(*(const uint16_t *)&msg[0]);
	const uint32_t *ack_bits = (const uint32_t *)&msg[2];
	uint32_t cur_bits = net_to_host_32(ack_bits[0]);

	assert(!((len - 2) & 3));
	len = (len - 2) / 4;

	/* L("--> processing ack %d/%d [%d:%s]", 0, len, ack, ack_bits_to_s(cur_bits)); */

	while (i < len) {
		if (!(cur_bits & (1 << biti))) {
			goto cont;
		} else if (!(ep = seq_buf_get(sent, ack - i))) {
			goto cont;
		} else if (ep->acked) {
			goto cont;
		}

		ep->acked = true;

		struct ack_msgs_cb_ctx ctx = {
			.msg = ep->msg, .msgs = ep->msgs,
			.acker = acker,
		};

		sack_iter(sk, &ctx, ack_msgs_cb);

cont:
		++biti;
		if (!(biti & 31)) {
			biti = 0;
			++i;
			cur_bits = net_to_host_32(ack_bits[i]);
			/* L("--> processing ack %d/%d [%d:%s]", i, len, ack, ack_bits_to_s(cur_bits)); */
		}
	}
}

#define ACK_BITS_BUFLEN 64

void
packet_write_acks(struct build_packet_ctx *bpc)
{
	static uint32_t ack_bits[ACK_BITS_BUFLEN], tmp32;
	const uint16_t start = bpc->cx->sb_recvd.head;
	uint16_t acks, tmp16, i;

	assert(seq_gt(bpc->cx->sb_recvd.head, bpc->cx->sb_recvd.last_acked));

	acks = bpc->cx->sb_recvd.head - bpc->cx->sb_recvd.last_acked;
	if (acks & 31) {
		acks = (acks / 32) + 1;
	} else {
		acks = acks / 32;
	}

	if (acks > ACK_BITS_BUFLEN) {
		LOG_W("overflowing ~%d acks", (acks - ACK_BITS_BUFLEN) * 32);
	}

	seq_buf_gen_ack_bits(&bpc->cx->sb_recvd, ack_bits, acks, start);

	tmp16 = host_to_net_16(start);
	packet_write(bpc, &tmp16, 2);

	for (i = 0; i < acks; ++i) {
		tmp32 = host_to_net_32(ack_bits[i]);
		packet_write(bpc, &tmp32, 4);
	}
}

void
packet_write_msg(struct build_packet_ctx *bpc, uint16_t id,
	void *itm, uint16_t len)
{
	struct msgr_transport_rudp_ctx *ctx = bpc->msgr->transport_ctx;
	struct sb_elem_packet *ep = seq_buf_get(&bpc->cx->sb_sent, bpc->seq);
	assert(ep);

	/* L("  msg %d", id); */

	ep->msg[ep->msgs] = id;
	++ep->msgs;

	if (ep->msgs > ctx->stats.packet_msg_count_max) {
		ctx->stats.packet_msg_count_max = ep->msgs;
	}
	++ctx->stats.messages_sent;

	packet_write(bpc, itm, len);
}

void
packet_write_setup(struct build_packet_ctx *bpc, uint16_t seq,
	enum packet_type type, enum packet_flags flags)
{
	uint16_t n;

	bpc->seq = seq;

	if (type == packet_type_normal) {
		struct sb_elem_packet *ep = seq_buf_insert(&bpc->cx->sb_sent, bpc->seq);
		assert(ep);
		*ep = (struct sb_elem_packet) { 0 };
	}

	n = host_to_net_16(seq);
	packet_write(bpc, &n, 2);

	n = host_to_net_16((flags << 8) | type);
	packet_write(bpc, &n, 2);

	/* L("*** %d %x- packet:%d", type, flags, seq); */
}

void
packet_read_hdr(const uint8_t *msg, struct packet_hdr *phdr)
{
	uint16_t n;

	memcpy(&phdr->seq, &msg[0], 2);
	phdr->seq  = net_to_host_16(phdr->seq);

	memcpy(&n, &msg[2], 2);
	n = net_to_host_16(n);
	phdr->flags = n >> 8;
	phdr->type = n & 0xff;
}

void
packet_write_hello(struct build_packet_ctx *bpc, uint16_t id)
{
	uint16_t n;

	n = host_to_net_16(id);
	packet_write(bpc, &n, 2);
}

void
packet_read_hello(const uint8_t *msg, struct packet_hello *ph)
{
	uint16_t n;
	memcpy(&n, &msg[0], 2);
	ph->id = net_to_host_16(n);
}
