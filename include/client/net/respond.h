#ifndef __NET_RESPOND_H
#define __NET_RESPOND_H
#include "server_cx.h"

void net_respond(struct server_cx *s);
void net_respond_init(uint32_t client_id);
#endif
