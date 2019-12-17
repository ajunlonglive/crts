#ifndef __NET_CONNECTION_H
#define __NET_CONNECTION_H
#include <stdlib.h>
#include <arpa/inet.h>

struct connection {
	union {
		struct sockaddr_in ia;
		struct sockaddr sa;
	} saddr;

	in_addr_t addr;
	in_port_t port;
	long stale;
	int motivator;
};

struct connection_pool {
	size_t len;
	size_t cap;
	int seq;
	struct connection *l;
};

void cx_inspect(const struct connection *c);
void cx_init(struct connection *c, const struct sockaddr_in *addr);
#endif
