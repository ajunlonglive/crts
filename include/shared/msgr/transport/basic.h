#ifndef SHARED_MSGR_TRANSPORT_BASIC_H
#define SHARED_MSGR_TRANSPORT_BASIC_H
#include "shared/msgr/msgr.h"

void msgr_transport_send_basic(struct msgr *msgr);
void msgr_transport_recv_basic(struct msgr *msgr);
void msgr_transport_queue_basic(struct msgr *msgr, enum message_type mt,
	void *msg, msg_addr_t dest);
#endif
