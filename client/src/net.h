#ifndef __NET_H
#define __NET_H
#include <arpa/inet.h>
#include "queue.h"

struct server {
	struct sockaddr_in listen_addr;
	struct sockaddr_in server_addr;
	int sock;

	struct queue *inbound;
};

void net_receive(struct server *s);
void net_respond(struct server *s);
struct server *net_connect(void);
#endif
