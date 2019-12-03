#ifndef __NET_POOL_H
#define __NET_POOL_H
#include <arpa/inet.h>
#include "types/hash.h"
#include "connection.h"

struct cx_pool {
	struct hash *cxs;
	struct {
		size_t len;
		size_t cap;
		struct connection *cxs;
	} mem;
};

struct connection *cx_establish(struct cx_pool *cp, struct sockaddr_in *addr);
void cx_prune(struct cx_pool *, long ms);
#endif
