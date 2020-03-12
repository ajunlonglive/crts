#ifndef SHARED_NET_RECV_MSGS_H
#define SHARED_NET_RECV_MSGS_H

#include "shared/net/defs.h"
#include "shared/net/connection.h"

typedef size_t (*msg_unpacker)(void *data, struct connection *, const char *buf);

struct recv_msgs_ctx {
	msg_unpacker unpacker;
	struct cx_pool *cxs;
	struct msg_queue *sent;
	struct darr *out;
	int sock;
};

void recv_msgs(const struct recv_msgs_ctx *ctx);
#endif
