#include "posix.h"

#include <string.h>

#include "shared/msgr/transport/rudp.h"
#include "shared/msgr/transport/rudp/cx_pool.h"
#include "shared/msgr/transport/rudp/queue.h"
#include "shared/msgr/transport/rudp/recv.h"
#include "shared/msgr/transport/rudp/send.h"
#include "shared/types/sack.h"
#include "shared/util/log.h"

#define BUFLEN 2048
#define MSG_BUFLEN (1024 * 1024)

/* init */

static size_t
pack_msg_wrapper(void *msg, uint8_t *buf, uint32_t blen)
{
	return pack_message(msg, buf, blen);
}


bool
msgr_transport_init_rudp(struct msgr_transport_rudp_ctx *ctx,
	struct msgr *msgr, const struct sock_impl *impl,
	struct sock_addr *bind_addr)
{
	msgr->transport_ctx = ctx;
	msgr->send = rudp_send;
	msgr->recv = rudp_recv;
	msgr->queue = rudp_queue;

	*ctx = (struct msgr_transport_rudp_ctx) { .si = impl };

	cx_pool_init(&ctx->pool);

	if (!ctx->si->bind(bind_addr, &ctx->sock)) {
		return false;
	}

	sack_init(&ctx->msg_sk_send, sizeof(struct msg_sack_hdr), MSG_BUFLEN,
		pack_msg_wrapper);

	return true;
}

void
rudp_connect(struct msgr *msgr, struct sock_addr *addr)
{
	struct msgr_transport_rudp_ctx *ctx = msgr->transport_ctx;

	if (!hdarr_get(&ctx->pool.cxs, addr)) {
		cx_add(&ctx->pool, addr, 0);
	}
}
