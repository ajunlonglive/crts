#ifndef SHARED_NET_SEND_MSG_H
#define SHARED_NET_SEND_MSG_H

#include "shared/net/defs.h"
#include "shared/net/msg_queue.h"

struct net_ctx;
void send_msgs(struct net_ctx *nx);
#endif
