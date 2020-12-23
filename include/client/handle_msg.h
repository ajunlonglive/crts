#ifndef CLIENT_HANDLE_MSG_H
#define CLIENT_HANDLE_MSG_H

#include "client/sim.h"
#include "shared/net/net_ctx.h"
#include "shared/sim/world.h"

void client_handle_msg(struct net_ctx *nx, enum message_type mt, void *_msg,
	struct connection *cx);
#endif
