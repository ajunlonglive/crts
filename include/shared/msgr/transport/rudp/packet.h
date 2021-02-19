#ifndef SHARED_MSGR_TRANSPORT_RUDP_PACKET_H
#define SHARED_MSGR_TRANSPORT_RUDP_PACKET_H

#include "shared/msgr/transport/rudp.h"

#define PACKET_MAX_LEN 1024
#define PACKET_HDR_LEN 10

struct packet_hdr {
	msg_seq_t seq;
	uint16_t sender_id;
	msg_seq_t ack;
	uint32_t ack_bits;
};

struct build_packet_ctx {
	struct msgr *msgr;
	struct rudp_cx *cx;
	char buf[PACKET_MAX_LEN];
	uint16_t bufi, seq;
};

bool packet_space_available(struct build_packet_ctx *bpc, uint16_t len);
void packet_write_msg(struct build_packet_ctx *bpc, uint16_t id,
	void *itm, uint16_t len);
void packet_write_setup(struct build_packet_ctx *bpc, uint16_t seq, uint16_t id);
void packet_read_hdr(const uint8_t *msg, struct packet_hdr *phdr);
void packet_ack_process(struct sack *sk, struct seq_buf *sent, uint16_t ack,
	uint32_t ack_bits, msg_addr_t acker);
#endif