#include "posix.h"

#include "shared/math/rand.h"
#include "shared/platform/sockets/dummy.h"
#include "shared/util/log.h"

struct sock_impl_dummy_conf sock_impl_dummy_conf = {
	.server = { .addr = 1, },
	.client = { .addr = 2, },
	.server_id   = 0xdead,
	.client_id   = 0xbeef,
	.reliability = 1.0,
};

enum dummy_dest {
	dest_server,
	dest_client,
	dest_err,
};

static enum dummy_dest
lookup_dest(struct sock_addr *dest)
{
	if (dest->addr == sock_impl_dummy_conf.client.addr) {
		return dest_client;
	} else if (dest->addr == sock_impl_dummy_conf.server.addr) {
		return dest_server;
	} else {
		LOG_W("unmatched dummy dest");
		return dest_err;
	}
}

static void
dsock_addr_init(struct sock_addr *addr, uint16_t port)
{
	/* NOOP */
}

static bool
dsock_resolve(struct sock_addr *addr, const char *host)
{
	return true;
}

static bool
dsock_bind(struct sock_addr *addr, sock_t *sock)
{
	return true;
}

static void
dsock_recv(sock_t sock, uint8_t *buf, uint32_t blen, void *ctx,
	sock_recv_cb cb)
{
	/* NOOP */
}

static bool
dsock_send(sock_t sock, uint8_t *buf, uint32_t blen,
	struct sock_addr *dest)
{
	if (drand48() > sock_impl_dummy_conf.reliability) {
		L("dropping :^)");
		return true;
	}

	switch (lookup_dest(dest)) {
	case dest_server:
		sock_impl_dummy_conf.cb(buf, blen,
			&sock_impl_dummy_conf.client,
			sock_impl_dummy_conf.server_ctx);
		return true;
	case dest_client:
		sock_impl_dummy_conf.cb(buf, blen,
			&sock_impl_dummy_conf.server,
			sock_impl_dummy_conf.client_ctx);
		return true;
	default:
		return false;
	}
}

const struct sock_impl sock_impl_dummy = {
	.addr_init = dsock_addr_init,
	.resolve = dsock_resolve,
	.bind = dsock_bind,
	.recv = dsock_recv,
	.send = dsock_send,
};
