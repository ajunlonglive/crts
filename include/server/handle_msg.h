#ifndef SERVER_HANDLE_MSG_H
#define SERVER_HANDLE_MSG_H

#include "server/sim/sim.h"
#include "shared/net/net_ctx.h"

void handle_msgs(struct simulation *sim, struct net_ctx *nx);
void handle_msgs_init(void);
#endif
