#ifndef SHARED_NET_SEND_MSG_H
#define SHARED_NET_SEND_MSG_H

#include "shared/net/defs.h"
#include "shared/net/msg_queue.h"

typedef size_t (*msg_packer)(const void *data, char *buf);

struct send_msgs_ctx {
	msg_packer packer;
	struct cx_pool *cxs;
	struct msg_queue *send;
	int sock;
};

void send_msgs(const struct send_msgs_ctx *ctx);
#endif
