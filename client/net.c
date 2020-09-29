#include "posix.h"

#include <netdb.h>
#include <string.h>

#include "client/handle_msg.h"
#include "client/net.h"
#include "shared/constants/port.h"
#include "shared/net/inet_aton.h"
#include "shared/net/net_ctx.h"
#include "shared/util/log.h"

static struct sockaddr_in server_addr = { 0 };

struct net_ctx *
net_init(const char *host, struct c_simulation *sim)
{
	struct net_ctx *nx;
	int ret;
	struct addrinfo *resp = NULL, hints = {
		.ai_family = AF_INET,
		.ai_socktype = SOCK_DGRAM,
	};

	nx = net_ctx_init(0, 0, handle_msg, sim->id);

	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(PORT);


	if ((ret = getaddrinfo(host, NULL, &hints, &resp)) != 0) {
		LOG_W("failed to resolve '%s': %s", host, strerror(ret));
		return NULL;
	}

	union {
		struct sockaddr *sa;
		struct sockaddr_in *ia;
	} saddr = { .sa = resp->ai_addr };

	L("resolved %s to %s", resp->ai_canonname, inet_ntoa(saddr.ia->sin_addr));

	server_addr.sin_addr = saddr.ia->sin_addr;

	nx->usr_ctx = sim;

	return nx;
}

void
check_add_server_cx(struct net_ctx *nx)
{
	if (hdarr_len(nx->cxs.cxs) == 0) {
		L("re-establishing server connection");
		cx_add(&nx->cxs, &server_addr, 0);
	}
}
