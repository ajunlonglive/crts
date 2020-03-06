#ifndef __NET_POOL_H
#define __NET_POOL_H
#include <arpa/inet.h>

#include "server/net/connection.h"
#include "shared/types/hash.h"

struct cx_pool {
	struct hdarr *cxs;
};

struct cx_pool *cx_pool_init(void);
void cx_prune(struct cx_pool *, long ms);
struct connection *cx_establish(struct cx_pool *cp, struct sockaddr_in *addr);
#endif
