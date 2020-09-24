#ifndef SERVER_NET_H
#define SERVER_NET_H

#include "server/sim/sim.h"
#include "shared/net/msg_queue.h"
#include "shared/serialize/message.h"

struct net_ctx * net_init(struct simulation *sim);
void broadcast_msg(struct net_ctx *nx, enum message_type t, void *dat,
	enum msg_flags f);
#endif
