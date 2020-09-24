#include "posix.h"

#include <string.h>

#include "client/handle_msg.h"
#include "client/net.h"
#include "shared/constants/port.h"
#include "shared/net/inet_aton.h"
#include "shared/net/net_ctx.h"
#include "shared/util/log.h"

static long outbound_id;
static struct sockaddr_in server_addr = { 0 };

struct net_ctx *
net_init(const char *ipv4addr, struct c_simulation *sim)
{
	struct net_ctx *nx;

	nx = net_ctx_init(0, 0, handle_msg);

	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(PORT);
	if (!inet_aton(ipv4addr, &server_addr.sin_addr)) {
		LOG_W("failed to parse address: '%s'", ipv4addr);
	}

	nx->usr_ctx = sim;

	return nx;
}

void
check_add_server_cx(struct net_ctx *nx)
{
	if (hdarr_len(nx->cxs.cxs) == 0) {
		L("re-establishing server connection");
		cx_establish(&nx->cxs, &server_addr);
	}
}

void
net_set_outbound_id(long id)
{
	outbound_id = id;
}
