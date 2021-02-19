#ifndef MSGR_TRANSPORT_RUDP_ACK_H
#define MSGR_TRANSPORT_RUDP_ACK_H

#include <stdbool.h>

#include "shared/msgr/transport/rudp/common.h"

#define RECVD_BUF_SIZE 1024
struct recvd_buf {
	struct {
		msg_seq_t seq;
	} buf[RECVD_BUF_SIZE];
};

struct packet_data *get_packet_data(msg_seq_t seq);
struct packet_data *set_packet_data(msg_seq_t seq);
#endif
