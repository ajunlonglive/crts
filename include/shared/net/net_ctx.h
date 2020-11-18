#ifndef SHARED_NET_NET_CXT_H
#define SHARED_NET_NET_CXT_H

#include "shared/net/msg_queue.h"
#include "shared/net/pool.h"
#include "shared/net/recv_msgs.h"
#include "shared/net/send_msgs.h"
#include "shared/serialize/message.h"

struct net_ctx;

typedef void ((*message_handler)(struct net_ctx *, enum message_type mt, void *msg, struct connection *cx));

struct net_ctx {
	void *usr_ctx;
	uint16_t id;

	message_handler handler;

	int sock;

	struct cx_pool cxs;
	struct msg_queue send;

	struct {
		struct message msg;
		cx_bits_t dest;
		enum msg_flags f;
	} buf;
};

void net_ctx_init(struct net_ctx *nx, uint32_t port, uint32_t addr, message_handler handler, uint16_t id);
void queue_msg(struct net_ctx *nx, enum message_type mt, void *msg, cx_bits_t dest,
	enum msg_flags f);
#endif
