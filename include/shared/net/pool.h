#ifndef SHARED_NET_POOL_H
#define SHARED_NET_POOL_H

#include "shared/net/defs.h"

struct cx_pool {
	struct hdarr *cxs;
	cx_bits_t cx_bits;
};

void cx_pool_init(struct cx_pool *);
void cx_prune(struct cx_pool *, long ms);
struct connection *cx_establish(struct cx_pool *cp, struct sockaddr_in *addr);
struct connection *cx_add(struct cx_pool *cp, struct sockaddr_in *addr, uint16_t id);
void cx_pool_clear(struct cx_pool *cp);
#endif
