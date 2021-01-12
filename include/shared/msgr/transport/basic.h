#ifndef SHARED_MSGR_TRANSPORT_BASIC_H
#define SHARED_MSGR_TRANSPORT_BASIC_H
#include "shared/msgr/msgr.h"

struct msgr_transport_basic_ctx {
	struct msg_sender self;
	struct msgr *msgr_dest;
};

void msgr_transport_init_basic(struct msgr *msgr, struct msgr *dest,
	struct msgr_transport_basic_ctx *ctx);
#endif
