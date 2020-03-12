#ifndef SHARED_NET_NET_CXT_H
#define SHARED_NET_NET_CXT_H

#include "shared/net/msg_queue.h"
#include "shared/net/pool.h"
#include "shared/net/recv_msgs.h"
#include "shared/net/send_msgs.h"

struct net_ctx {
	struct send_msgs_ctx smc;
	struct recv_msgs_ctx rmc;

	struct cx_pool cxs;
	struct msg_queue *send;
	struct darr *recvd;
};

struct net_ctx *net_ctx_init(uint32_t port, uint32_t addr, size_t send_size,
	size_t recv_size, msg_unpacker unpacker, msg_packer packer);
void net_respond(struct net_ctx *s);
void net_receive(struct net_ctx *s);
#endif
