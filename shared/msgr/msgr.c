#include "posix.h"

#include <string.h>

#include "shared/msgr/msgr.h"
#include "shared/util/log.h"

static void
flush_msg_buf(struct msgr *msgr)
{
	if (!msgr->msg_buf.msg.mt) {
		return;
	}

	msgr->queue(msgr, &msgr->msg_buf.msg, msgr->msg_buf.dest);
	memset(&msgr->msg_buf.msg, 0, sizeof(struct message));
}

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
	flush_msg_buf(msgr);
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
	/* L("queueing  %s", inspect_message(mt, msg)); */

	bool appended = msgr->msg_buf.msg.count
			&& msgr->msg_buf.msg.mt == mt
			&& msgr->msg_buf.dest == dest
			&& append_msg(&msgr->msg_buf.msg, msg);

	if (!appended) {
		flush_msg_buf(msgr);

		msgr->msg_buf.msg.mt = mt;
		msgr->msg_buf.dest = dest;

		if (!append_msg(&msgr->msg_buf.msg, msg)) {
			LOG_W("failed to append message");
			assert(false);
		}
	}
}
