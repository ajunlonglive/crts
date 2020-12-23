#ifndef SERVER_HANDLE_MSG_H
#define SERVER_HANDLE_MSG_H

#include "server/sim/sim.h"
#include "shared/net/net_ctx.h"

void server_handle_msg(struct net_ctx *nx, enum message_type mt, void *_msg,
	struct connection *cx);
#endif
