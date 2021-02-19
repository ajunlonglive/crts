#ifndef SHARED_MSGR_TRANSPORT_RUDP_RECV_H
#define SHARED_MSGR_TRANSPORT_RUDP_RECV_H

#include "shared/msgr/transport/rudp.h"

void rudp_recv(struct msgr *msgr);
void rudp_recv_cb(uint8_t *msg, uint32_t len,
	const struct sock_addr *sender_addr, void *_ctx);
#endif
