#ifndef SHARED_PLATFORM_COMMON_SOCKETS_H
#define SHARED_PLATFORM_COMMON_SOCKETS_H

#include <stdbool.h>
#include <stdint.h>

typedef uint16_t sock_t;

struct sock_addr {
	uint32_t addr;
	uint16_t port;
};

typedef void (*sock_recv_cb)(uint8_t *msg, uint32_t bytes,
	const struct sock_addr *sender, void *ctx);

typedef bool (*sock_init)(void);
typedef bool (*sock_deinit)(void);
typedef void (*sock_addr_init)(struct sock_addr *addr, uint16_t port);
typedef bool (*sock_resolve)(struct sock_addr *addr, const char *host);
typedef bool (*sock_bind)(struct sock_addr *addr, sock_t *sock);
typedef void (*sock_recv)(sock_t sock, uint8_t *buf, uint32_t blen,
	void *ctx, sock_recv_cb cb);
typedef bool (*sock_send)(sock_t sock, uint8_t *buf, uint32_t blen,
	struct sock_addr *dest);

struct sock_impl {
	sock_init init;
	sock_deinit deinit;
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
const char *sock_addr_to_s(const struct sock_addr *addr);

struct sock_impl_dummy_conf {
	struct sock_addr client, server;
	uint32_t client_id, server_id;
	sock_recv_cb cb;
	void *client_ctx, *server_ctx;
	double reliability;
};

extern struct sock_impl_dummy_conf sock_impl_dummy_conf;

#ifndef NDEBUG
extern bool socket_reliability_set;
extern double socket_reliability;
#endif
#endif
