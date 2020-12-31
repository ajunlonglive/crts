#include "posix.h"

#include "shared/msgr/transport/basic.h"
#include "shared/util/log.h"

void
msgr_transport_send_basic(struct msgr *msgr)
{
	/* NOOP */
}

void
msgr_transport_recv_basic(struct msgr *msgr)
{
	/* NOOP */
}

void
msgr_transport_queue_basic(struct msgr *msgr, enum message_type mt,
	void *msg, msg_addr_t dest)
{
	static struct msg_sender self = { .flags = msf_first_message };
	struct msgr *msgr_dest = msgr->transport_ctx;

	if (!(self.flags & msf_first_message)) {
		self.id = msgr->id;
		msgr_dest->handler(msgr_dest, mt, msg, &self);
	} else {
		self.id = msgr->id;
		msgr_dest->handler(msgr_dest, mt, msg, &self);
		self.flags &= ~msf_first_message;
	}
}
