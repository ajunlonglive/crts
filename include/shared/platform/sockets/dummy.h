#ifndef SHARED_PLATFORM_SOCKETS_DUMMY_H
#define SHARED_PLATFORM_SOCKETS_DUMMY_H
#include "shared/platform/sockets/common.h"

struct sock_impl_dummy_conf {
	struct sock_addr client, server;
	uint32_t client_id, server_id;
	sock_recv_cb cb;
	void *client_ctx, *server_ctx;
	double reliability;
};

extern struct sock_impl_dummy_conf sock_impl_dummy_conf;
extern const struct sock_impl sock_impl_dummy;
#endif
