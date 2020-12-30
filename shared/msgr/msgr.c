#include "posix.h"

#include "shared/msgr/msgr.h"
#include "shared/msgr/transport/basic.h"

void
msgr_init(struct msgr *msgr, void *usr_ctx, msg_handler handler, uint16_t id)
{
	*msgr = (struct msgr) {
		.usr_ctx = usr_ctx,
		.handler = handler,
		.id = id
	};
}

void
msgr_send(struct msgr *msgr)
{
	msgr->send(msgr);
}

void
msgr_recv(struct msgr *msgr)
{
	msgr->recv(msgr);
}

void
msgr_queue(struct msgr *msgr, enum message_type mt,
	void *msg, msg_addr_t dest)
{
	msgr->queue(msgr, mt, msg, dest);
}

void
msgr_transport_init_basic(struct msgr *msgr, struct msgr *dest)
{
	msgr->send = msgr_transport_send_basic;
	msgr->recv = msgr_transport_recv_basic;
	msgr->queue = msgr_transport_queue_basic;
	msgr->transport_ctx = dest;
}
