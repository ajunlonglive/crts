#include "posix.h"

#include "shared/msgr/transport/rudp.h"
#include "shared/msgr/transport/rudp/cx_pool.h"
#include "shared/types/sack.h"
#include "shared/util/log.h"

#define BUFLEN 2048
#define MSG_BUFLEN (1024 * 1024)

typedef uint16_t msg_seq_t;

struct msg_sack_hdr {
	msg_addr_t dest;
};

struct packet_hdr {
	msg_seq_t seq;
	uint16_t id;
};

/* send */

static enum del_iter_result
send_cb(void *_ctx, void *_hdr, void *itm, uint16_t len)
{
	struct msgr *msgr = _ctx;
	struct msgr_transport_rudp_ctx *ctx = msgr->transport_ctx;
	struct msg_sack_hdr *hdr = _hdr;
	struct rudp_cx *cx;

	uint32_t i;
	for (i = 0; i < hdarr_len(&ctx->pool.cxs); ++i) {
		L("sending %d to cx:%d", len, i);

		cx = hdarr_get_by_i(&ctx->pool.cxs, i);
		if (!(hdr->dest & cx->addr)) {
			continue;
		}

		/* unpack_message(itm, len, tmp_inspect_unpack_cb, NULL); */
		ctx->si->send(ctx->sock, itm, len, &cx->sock_addr);
	}

	return dir_del;
}

static void
msgr_transport_send_rudp(struct msgr *msgr)
{
	struct msgr_transport_rudp_ctx *ctx = msgr->transport_ctx;

	sack_iter(&ctx->msg_sk, msgr, send_cb);
}

/* recv */

struct unpack_ctx {
	struct msg_sender sender;
	struct msgr *msgr;
};

static void
unpack_cb(void *_ctx, enum message_type mt, void *msg)
{
	struct unpack_ctx *ctx = _ctx;
	ctx->msgr->handler(ctx->msgr, mt, msg, &ctx->sender);
	ctx->sender.flags &= ~msf_first_message;
}

void
rudp_recv_cb(uint8_t *msg, uint32_t len,
	const struct sock_addr *sender_addr, void *_ctx)
{
	L("recving %d", len);

	struct msgr *msgr = _ctx;
	struct msgr_transport_rudp_ctx *ctx = msgr->transport_ctx;
	struct rudp_cx *cx;
	struct unpack_ctx uctx = { .msgr = msgr };

	if (!(cx = cx_get(&ctx->pool, sender_addr))) {
		cx = cx_add(&ctx->pool, sender_addr, 99);
		uctx.sender.flags |= msf_first_message;
	}

	uctx.sender.addr = cx->addr;
	uctx.sender.id = cx->id;

	/* unpack_packet_hdr(msg, blen - hdrlen, unpack_cb, &uctx); */
	unpack_message(msg, len, unpack_cb, &uctx);
}

static void
msgr_transport_recv_rudp(struct msgr *msgr)
{
	struct msgr_transport_rudp_ctx *ctx = msgr->transport_ctx;
	/* It is OK this is static since recv isnt used when we are using the
	 * dummy impl */
	static uint8_t buf[BUFLEN] = { 0 };

	ctx->si->recv(ctx->sock, buf, BUFLEN, msgr, rudp_recv_cb);

	cx_prune(&ctx->pool, 10);
}

/* queue */

static size_t
pack_msg_wrapper(void *msg, uint8_t *buf, uint32_t blen)
{
	return pack_message(msg, buf, blen);
}

static void
msgr_transport_queue_rudp(struct msgr *msgr, struct message *msg,
	msg_addr_t dest)
{
	struct msgr_transport_rudp_ctx *ctx = msgr->transport_ctx;

	if (!dest) {
		if (!ctx->pool.used_addrs) {
			return;
		}

		dest = ctx->pool.used_addrs;
	}

	struct msg_sack_hdr hdr = { .dest = dest };

	/* L("sending ~ %s", inspect_message(msg->mt, msg)); */
	sack_stuff(&ctx->msg_sk, &hdr, msg);
}

/* init */

bool
msgr_transport_init_rudp(struct msgr_transport_rudp_ctx *ctx,
	struct msgr *msgr, const struct sock_impl *impl,
	struct sock_addr *bind_addr)
{
	msgr->transport_ctx = ctx;
	msgr->send = msgr_transport_send_rudp;
	msgr->recv = msgr_transport_recv_rudp;
	msgr->queue = msgr_transport_queue_rudp;

	*ctx = (struct msgr_transport_rudp_ctx) { .si = impl };

	cx_pool_init(&ctx->pool);

	if (!ctx->si->bind(bind_addr, &ctx->sock)) {
		return false;
	}

	sack_init(&ctx->msg_sk, sizeof(struct msg_sack_hdr), MSG_BUFLEN,
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
