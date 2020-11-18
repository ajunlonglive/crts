#ifndef SERVER_NET_H
#define SERVER_NET_H

#include "server/sim/sim.h"
#include "shared/net/msg_queue.h"
#include "shared/net/net_ctx.h"
#include "shared/serialize/message.h"

void net_init(struct simulation *sim, struct net_ctx *nx);
void broadcast_msg(struct net_ctx *nx, enum message_type t, void *dat,
	enum msg_flags f);
#endif
