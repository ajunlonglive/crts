#ifndef __NET_RECEIVE_H
#define __NET_RECEIVE_H
#include "server_cx.h"

void net_receive_init(void);
void net_receive(struct server_cx *s);
#endif
