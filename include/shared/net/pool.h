#ifndef __NET_POOL_H
#define __NET_POOL_H

#include "shared/net/defs.h"

struct cx_pool {
	struct hdarr *cxs;
	cx_bits_t cx_bits;
};

void cx_pool_init(struct cx_pool *);
void cx_prune(struct cx_pool *, long ms);
struct connection *cx_establish(struct cx_pool *cp, struct sockaddr_in *addr);
#endif
