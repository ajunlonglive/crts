#ifndef SHARED_MSGR_TRANPOSRT_RUDP_CX_POOL_H
#define SHARED_MSGR_TRANPOSRT_RUDP_CX_POOL_H

#include "shared/msgr/msgr.h"
#include "shared/msgr/transport/rudp/seq_buf.h"
#include "shared/platform/sockets/common.h"
#include "shared/types/hdarr.h"

#define MAX_CXS 32

#define RECVD_BUF_SIZE 1024
#define RECVD_BUF_MOD 0x3ff

typedef uint16_t msg_seq_t;

struct rudp_cx {
	struct seq_buf sb_sent, sb_recvd;
	msg_seq_t local_seq;
	struct sock_addr sock_addr;
	uint32_t stale;
	msg_addr_t addr;
	uint16_t id;
};

struct cx_pool {
	struct hdarr cxs;
	msg_addr_t used_addrs;
};

void cx_init(struct rudp_cx *c, const struct sock_addr *addr);
void cx_destroy(struct rudp_cx *c);

void cx_pool_init(struct cx_pool *);
void cx_prune(struct cx_pool *, long ms);
struct rudp_cx *cx_get(struct cx_pool *cp, const struct sock_addr *addr);
struct rudp_cx *cx_add(struct cx_pool *cp, const struct sock_addr *addr,
	uint16_t id);
void cx_pool_clear(struct cx_pool *cp);
#endif
