#ifndef __NET_H
#define __NET_H
#include <stdlib.h>
#include <arpa/inet.h>
#include "queue.h"

struct client {
	in_addr_t addr;
	in_port_t port;
	int stale;
	int motivator;
};

struct clients {
	size_t len;
	size_t cap;
	int next_motivator;
	struct client *l;
};

struct server {
	struct sockaddr_in addr;
	int sock;

	struct clients cxs;
	struct queue *outbound;
	struct queue *inbound;
};

struct server *server_init(void);
void net_receive(struct server *s);
void net_respond(struct server *s);
#endif
