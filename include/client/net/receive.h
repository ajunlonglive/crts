#ifndef __NET_RECEIVE_H
#define __NET_RECEIVE_H

#include <stdbool.h>

#include "client/net/server_cx.h"

void net_receive_init(void);
bool net_receive(struct server_cx *s);
#endif
