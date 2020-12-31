#ifndef CLIENT_HANDLE_MSG_H
#define CLIENT_HANDLE_MSG_H

#include "shared/msgr/msgr.h"
#include "shared/sim/world.h"

void client_handle_msg(struct msgr *msgr, enum message_type mt, void *_msg,
	struct msg_sender *sender);
#endif
