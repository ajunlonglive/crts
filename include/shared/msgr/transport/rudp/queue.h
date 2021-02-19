#ifndef SHARED_MSGR_TRANSPORT_RUDP_QUEUE_H
#define SHARED_MSGR_TRANSPORT_RUDP_QUEUE_H

#include "shared/msgr/transport/rudp.h"

void rudp_queue(struct msgr *msgr, struct message *msg,
	msg_addr_t dest);
#endif
