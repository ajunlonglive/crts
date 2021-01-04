#ifndef SHARED_MSGR_TRANPOSRT_RUDP_CX_POOL_H
#define SHARED_MSGR_TRANPOSRT_RUDP_CX_POOL_H

#include "shared/msgr/transport/rudp.h"
#include "shared/types/hdarr.h"

#define MAX_CXS 32

struct rudp_cx {
	/* struct hash acks; */
	struct sock_addr sock_addr;
	uint32_t stale;
	msg_addr_t addr;
	uint16_t id;
};

struct cx_pool {
	struct hdarr cxs;
	msg_addr_t used_addrs;
};


void cx_inspect(const struct rudp_cx *c);
void cx_init(struct rudp_cx *c, const struct sock_addr *addr);
void cx_destroy(struct rudp_cx *c);

void cx_pool_init(struct cx_pool *);
void cx_prune(struct cx_pool *, long ms);
struct rudp_cx *cx_get(struct cx_pool *cp, const struct sock_addr *addr);
struct rudp_cx *cx_add(struct cx_pool *cp, const struct sock_addr *addr,
	uint16_t id);
void cx_pool_clear(struct cx_pool *cp);
#endif
