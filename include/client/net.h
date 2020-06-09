#ifndef CLIENT_NET_H
#define CLIENT_NET_H

#include "shared/messaging/client_message.h"
#include "shared/net/msg_queue.h"

struct net_ctx *net_init(const char *ipv4addr);
void send_msg(struct net_ctx *nx, enum client_message_type t, const void *dat,
	enum msg_flags f);
void net_set_outbound_id(long id);
void check_add_server_cx(struct net_ctx *nx);
#endif
