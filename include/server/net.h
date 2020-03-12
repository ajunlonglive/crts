#ifndef SERVER_NET_H
#define SERVER_NET_H

#include "shared/messaging/client_message.h"
#include "shared/messaging/server_message.h"

struct wrapped_message {
	struct client_message cm;
	struct connection *cx;
};

struct net_ctx *net_init(void);
void send_msg(struct net_ctx *nx, enum server_message_type t, const void *dat);
#endif
