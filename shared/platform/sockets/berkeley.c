#include "posix.h"

#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <string.h>

#include "shared/platform/sockets/berkeley.h"
#include "shared/util/log.h"

union saddr {
	struct sockaddr sa;
	struct sockaddr_in in;
};

static const socklen_t socklen = sizeof(struct sockaddr_in);

static void
bsock_addr_init(struct sock_addr *addr, uint16_t port)
{
	*addr = (struct sock_addr) {
		.port = htons(port)
	};
}

static bool
bsock_resolve(struct sock_addr *addr, const char *host)
{
	int ret;
	struct addrinfo *resp = NULL, hints = {
		.ai_family = AF_INET,
		.ai_socktype = SOCK_DGRAM,
	};

	if ((ret = getaddrinfo(host, NULL, &hints, &resp)) != 0) {
		LOG_W("failed to resolve '%s': %s", host, strerror(ret));
		return false;
	}

	union {
		struct sockaddr *sa;
		struct sockaddr_in *ia;
	} saddr = { .sa = resp->ai_addr };

	L("resolved %s to %s", resp->ai_canonname, inet_ntoa(saddr.ia->sin_addr));

	/* addr->port = htons(port); */
	addr->addr = saddr.ia->sin_addr.s_addr;

	freeaddrinfo(resp);
	return true;
}

static bool
bsock_bind(struct sock_addr *addr, sock_t *sock)
{
	int flags, ret;
	union saddr saddr = {
		.in = {
			.sin_family = AF_INET,
			.sin_port = addr->port,
			.sin_addr = {
				.s_addr = addr->addr
			},
		}
	};

	int isock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (isock < 0) {
		L("failed to create socket: %s", strerror(errno));
		return false;
	}

	assert(isock < UINT16_MAX);
	*sock = isock;

	if ((ret = bind(*sock, &saddr.sa, socklen)) != 0) {
		L("failed to bind socket: %s", strerror(errno));
		return false;
	}

	L("successfully bound socket @ %s", sock_addr_to_s(addr));

	flags = fcntl(*sock, F_GETFL);
	fcntl(*sock, F_SETFL, flags | O_NONBLOCK);

	return true;
}

static void
bsock_recv(sock_t sock, uint8_t *buf, uint32_t blen, void *ctx, sock_recv_cb cb)
{
	ssize_t res;
	union saddr caddr;
	struct sock_addr sender = { 0 };
	socklen_t caddrlen = socklen;

	while ((res = recvfrom(sock, buf, blen, 0, &caddr.sa, &caddrlen)) > 0) {
		assert(caddrlen == socklen);
		sender.port = caddr.in.sin_port;
		sender.addr = caddr.in.sin_addr.s_addr;

		cb(buf, res, &sender, ctx);
	}
}

static bool
bsock_send(sock_t sock, uint8_t *buf, uint32_t blen,
	struct sock_addr *dest)
{
	ssize_t res;
	union saddr destaddr = {
		.in = {
			.sin_family = AF_INET,
			.sin_port = dest->port,
			.sin_addr = { .s_addr = dest->addr },
		}
	};

	if ((res = sendto(sock, buf, blen, 0, &destaddr.sa, socklen)) <= 0) {
		return false;
	}

	return true;
}

const struct sock_impl sock_impl_system = {
	.addr_init = bsock_addr_init,
	.resolve = bsock_resolve,
	.bind = bsock_bind,
	.recv = bsock_recv,
	.send = bsock_send,
};
