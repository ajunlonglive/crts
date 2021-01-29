#ifndef SHARED_MSGR_TRANSPORT_RUDP_H
#define SHARED_MSGR_TRANSPORT_RUDP_H

#include "shared/msgr/msgr.h"
#include "shared/msgr/transport/rudp/cx_pool.h"
#include "shared/platform/sockets/common.h"
#include "shared/types/sack.h"

struct msgr_transport_rudp_ctx {
	struct sack msg_sk;
	const struct sock_impl *si;
	struct cx_pool pool;
	sock_t sock;
};

void rudp_recv_cb(uint8_t *msg, uint32_t len,
	const struct sock_addr *sender_addr, void *_ctx);
void rudp_connect(struct msgr *msgr, struct sock_addr *addr);
bool msgr_transport_init_rudp(struct msgr_transport_rudp_ctx *ctx,
	struct msgr *msgr, const struct sock_impl *impl,
	struct sock_addr *bind_addr);
#endif
