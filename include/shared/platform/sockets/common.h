#ifndef SHARED_PLATFORM_SOCKETS_COMMON_H
#define SHARED_PLATFORM_SOCKETS_COMMON_H

#include <stdbool.h>
#include <stdint.h>

typedef uint16_t sock_t;

struct sock_addr {
	uint32_t addr;
	uint16_t port;
};

typedef void (*sock_recv_cb)(uint8_t *msg, uint32_t bytes,
	const struct sock_addr *sender, void *ctx);

typedef void (*sock_addr_init)(struct sock_addr *addr, uint16_t port);
typedef bool (*sock_resolve)(struct sock_addr *addr, const char *host);
typedef bool (*sock_bind)(struct sock_addr *addr, sock_t *sock);
typedef void (*sock_recv)(sock_t sock, uint8_t *buf, uint32_t blen,
	void *ctx, sock_recv_cb cb);
typedef bool (*sock_send)(sock_t sock, uint8_t *buf, uint32_t blen,
	struct sock_addr *dest);

struct sock_impl {
	sock_addr_init addr_init;
	sock_resolve resolve;
	sock_bind bind;
	sock_recv recv;
	sock_send send;
};

enum sock_impl_type {
	sock_impl_type_system,
	sock_impl_type_dummy,
};

const struct sock_impl *get_sock_impl(enum sock_impl_type type);
#endif
