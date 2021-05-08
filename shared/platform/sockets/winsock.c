#include "posix.h"

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0600
#endif

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>

#include <strsafe.h>
#include <winsock2.h>
#include <ws2tcpip.h>

#include "shared/math/rand.h"
#include "shared/platform/sockets/winsock.h"
#include "shared/util/log.h"
#include "shared/util/mem.h"

union saddr {
	struct sockaddr sa;
	struct sockaddr_in in;
};

static const socklen_t socklen = sizeof(struct sockaddr_in);

#ifndef NDEBUG
bool socket_reliability_set = false;
double socket_reliability = 0.0;
#endif

#define ERR_BUF_LEN 64

static const char *
win_strerror(void)
{
	static char buf[ERR_BUF_LEN + 1] = { 0 };
	uint32_t err = GetLastError();

	if (!FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL, err, 0, buf, ERR_BUF_LEN, NULL)) {
		snprintf(buf, ERR_BUF_LEN, "error code %d", err);
	}

	return buf;
}

static bool
wsock_init(void)
{
	WSADATA d;
	if (WSAStartup(MAKEWORD(2, 2), &d)) {
		LOG_W("WSAStartup failed");
		return false;
	}

	return true;
}

static bool
wsock_deinit(void)
{
	WSACleanup();
	return true;
}

static void
wsock_addr_init(struct sock_addr *addr, uint16_t port)
{
	*addr = (struct sock_addr) {
		.port = htons(port)
	};
}

static bool
wsock_resolve(struct sock_addr *addr, const char *host)
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
wsock_bind(struct sock_addr *addr, sock_t *sock)
{
	int ret;
	union saddr saddr = {
		.in = {
			.sin_family = AF_INET,
			.sin_port = addr->port,
			.sin_addr = {
				.s_addr = addr->addr
			},
		}
	};

	SOCKET isock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (isock == INVALID_SOCKET) {
		L("failed to create socket: %s", win_strerror());
		return false;
	}

	assert(isock < UINT16_MAX);
	*sock = isock;

	if ((ret = bind(*sock, &saddr.sa, socklen)) == SOCKET_ERROR) {
		L("failed to bind socket: %s", win_strerror());
		return false;
	}

	L("successfully bound socket @ %s", sock_addr_to_s(addr));

	u_long non_block = 1;
	if (ioctlsocket(*sock, FIONBIO, &non_block) == SOCKET_ERROR) {
		L("failed to set nonblocking mode: %s", win_strerror());
		return false;
	}

	return true;
}

static void
wsock_recv(sock_t sock, uint8_t *buf, uint32_t blen, void *ctx, sock_recv_cb cb)
{
	ssize_t res;
	union saddr caddr;
	struct sock_addr sender = { 0 };
	socklen_t caddrlen = socklen;

	while ((res = recvfrom(sock, (char *)buf, blen, 0, &caddr.sa, &caddrlen)) > 0) {
		assert(caddrlen == socklen);
		sender.port = caddr.in.sin_port;
		sender.addr = caddr.in.sin_addr.s_addr;

		cb(buf, res, &sender, ctx);
	}
}

static bool
wsock_send(sock_t sock, uint8_t *buf, uint32_t blen,
	struct sock_addr *dest)
{
#ifndef NDEBUG
	if (socket_reliability_set && drand48() > socket_reliability) {
		return true;
	}
#endif

	ssize_t res;
	union saddr destaddr = {
		.in = {
			.sin_family = AF_INET,
			.sin_port = dest->port,
			.sin_addr = { .s_addr = dest->addr },
		}
	};

	if ((res = sendto(sock, (const char *)buf, blen, 0, &destaddr.sa, socklen)) <= 0) {
		return false;
	}

	return true;
}

const struct sock_impl sock_impl_system = {
	.init = wsock_init,
	.deinit = wsock_deinit,
	.addr_init = wsock_addr_init,
	.resolve = wsock_resolve,
	.bind = wsock_bind,
	.recv = wsock_recv,
	.send = wsock_send,
};
