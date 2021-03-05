#ifndef SHARED_MSGR_TRANSPORT_RUDP_PACKET_H
#define SHARED_MSGR_TRANSPORT_RUDP_PACKET_H

#include "shared/msgr/transport/rudp.h"

#define PACKET_MAX_LEN 1024
#define PACKET_HDR_LEN 4

enum packet_type {
	packet_type_normal,
	packet_type_ack,
	packet_type_connect,
};

enum packet_flags {
	packet_flag_do_ack = 1 << 0,
};

struct packet_hdr {
	msg_seq_t seq;
	enum packet_type type;
	enum packet_flags flags;
};

struct packet_hello {
	uint16_t id;
};

struct build_packet_ctx {
	struct msgr *msgr;
	struct rudp_cx *cx;
	char buf[PACKET_MAX_LEN];
	uint16_t bufi, seq, sent_packets, sent_msgs;
};

void packet_seq_buf_init(struct seq_buf *sb);

bool packet_space_available(struct build_packet_ctx *bpc, uint16_t len);
void packet_write_msg(struct build_packet_ctx *bpc, uint16_t id,
	void *itm, uint16_t len, bool record);


void packet_write_setup(struct build_packet_ctx *bpc, uint16_t seq,
	enum packet_type type, enum packet_flags flags);
void packet_read_hdr(const uint8_t *msg, struct packet_hdr *phdr);

void packet_write_acks(struct build_packet_ctx *bpc);
void packet_read_acks_and_process(struct sack *sk, struct seq_buf *sent,
	const uint8_t *msg, uint32_t len, msg_addr_t acker);

void packet_write_hello(struct build_packet_ctx *bpc, uint16_t id);
void packet_read_hello(const uint8_t *msg, struct packet_hello *ph);
#endif
