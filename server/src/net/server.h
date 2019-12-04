#ifndef __NET_H
#define __NET_H
#include <stdlib.h>
#include <arpa/inet.h>
#include "connection.h"
#include "pool.h"

struct server {
	int sock;

	struct cx_pool *cxs;
	struct queue *outbound;
	struct queue *inbound;
};

extern socklen_t socklen;

struct server *server_init(void);
void net_receive(struct server *s);
void net_respond(struct server *s);
#endif
