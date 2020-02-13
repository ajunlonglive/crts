#ifndef __NET_POOL_H
#define __NET_POOL_H
#include <arpa/inet.h>

#include "server/net/connection.h"
#include "shared/types/hash.h"

struct cx_pool {
	struct hash *cxs;
	struct {
		size_t len;
		size_t cap;
		struct connection *cxs;
	} mem;
};

struct cx_pool *cx_pool_init(void);
void cx_prune(struct cx_pool *, long ms);
const struct connection *cx_establish(struct cx_pool *cp, struct sockaddr_in *addr);
#endif
