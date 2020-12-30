#ifndef SERVER_HANDLE_MSG_H
#define SERVER_HANDLE_MSG_H

#include "server/sim/sim.h"
/* #include "shared/net/net_ctx.h" */
#include "shared/msgr/msgr.h"

void server_handle_msg(struct msgr *msgr, enum message_type mt, void *_msg,
	struct msg_sender *sender);
#endif
