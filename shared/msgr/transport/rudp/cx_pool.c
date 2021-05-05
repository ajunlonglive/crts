#include "posix.h"

#include <string.h>

#include "shared/msgr/transport/rudp/packet.h"
#include "shared/msgr/transport/rudp/cx_pool.h"
#include "shared/util/log.h"

//ms before remove connection
#define STALE_THRESHOLD 1000

static void
cx_inspect(const struct rudp_cx *cx)
{
	L("cx@%p %s | bit: %x age: %u",
		(void *)cx,
		sock_addr_to_s(&cx->sock_addr),
		cx->sender.addr,
		cx->stale
		);
}

void
cx_init(struct rudp_cx *cx, const struct sock_addr *addr)
{
	*cx = (struct rudp_cx) {
		.sock_addr = *addr,
	};

	packet_seq_buf_init(&cx->sb_sent);
	packet_seq_buf_init(&cx->sb_recvd);
}

void
cx_destroy(struct rudp_cx *cx)
{
	seq_buf_destroy(&cx->sb_sent);
	seq_buf_destroy(&cx->sb_recvd);
}

static const void *
connection_key_getter(void *_cx)
{
	struct rudp_cx *cx = _cx;

	return &cx->sender.addr;
}

void
cx_pool_init(struct cx_pool *cp)
{
	hdarr_init(&cp->cxs, MAX_CXS * 2, sizeof(struct sock_addr),
		sizeof(struct rudp_cx), connection_key_getter);
}

static bool
get_available_addr(struct cx_pool *cp, struct rudp_cx *cx)
{
	size_t i;
	msg_addr_t b;

	for (i = 0; i < MAX_CXS; ++i) {
		b = 1 << i;
		if (!(cp->used_addrs & b)) {
			cp->used_addrs |= b;
			cx->sender.addr = b;
			cx->addr_idx = i;
			return true;
		}
	}

	LOG_W("cxs full");
	return false;
}

struct rudp_cx *
cx_add(struct cx_pool *cp, const struct sock_addr *sock_addr, uint16_t id)
{
	struct rudp_cx cx;

	cx_init(&cx, sock_addr);
	cx.sender.id = id;

	if (!get_available_addr(cp, &cx)) {
		return NULL;
	}

	L("new connection");
	cx_inspect(&cx);

	hdarr_set(&cp->cxs, sock_addr, &cx);
	return hdarr_get(&cp->cxs, sock_addr);
}

struct rudp_cx *
cx_get(struct cx_pool *cp, const struct sock_addr *addr)
{
	struct rudp_cx *cl;

	if (!(cl = hdarr_get(&cp->cxs, addr))) {
		return NULL;
	}

	cl->stale = 0;

	return cl;
}

void
cx_prune(struct cx_pool *cp, long ms)
{
	struct rudp_cx *cx, *remove = NULL;

	uint32_t i;
	for (i = 0; i < hdarr_len(&cp->cxs); ++i) {
		cx = hdarr_get_by_i(&cp->cxs, i);
		cx->stale += ms;

		if (cx->stale >= STALE_THRESHOLD) {
			remove = cx;
		}
	}

	if (remove) {
		L("lost connection");
		cx_inspect(cx);
		cx_destroy(cx);

		cp->used_addrs &= ~cx->sender.addr;

		hdarr_del(&cp->cxs, &cx->sock_addr);
	}
}

void
cx_pool_clear(struct cx_pool *cp)
{
	hdarr_clear(&cp->cxs);
	cp->used_addrs = 0;
}
