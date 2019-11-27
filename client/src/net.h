#ifndef __NET_H
#define __NET_H
#include <arpa/inet.h>
#include "queue.h"

struct cxinfo {
	struct sockaddr_in listen_addr;
	struct sockaddr_in server_addr;
	int sock;

	struct queue *inbound;
	struct queue *outbound;

	int *run;
};

void net_receive(struct cxinfo *s);
void net_respond(struct cxinfo *s);
struct cxinfo *net_connect(const char *ipv4addr);
#endif
