#ifndef CLIENT_NET_H
#define CLIENT_NET_H
#include "shared/messaging/client_message.h"

struct net_ctx *net_init(const char *ipv4addr);
void send_msg(struct net_ctx *nx, enum client_message_type t, const void *dat);
void net_set_outbound_id(long id);
#endif
