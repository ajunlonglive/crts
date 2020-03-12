#include <stdlib.h>
#include <string.h>

#include "shared/net/bind_sock.h"
#include "shared/net/net_ctx.h"
#include "shared/types/darr.h"

struct net_ctx *
net_ctx_init(uint32_t port, uint32_t addr, size_t send_size, size_t recv_size,
	msg_unpacker unpacker, msg_packer packer)
{
	struct net_ctx *nx = calloc(1, sizeof(struct net_ctx));
	struct sockaddr_in ia;

	memset(&ia, 0, socklen);
	ia.sin_port = htons(port);
	ia.sin_addr.s_addr = htonl(addr);
	nx->rmc.sock = nx->smc.sock = bind_sock(&ia);

	cx_pool_init(&nx->cxs);
	nx->rmc.cxs = nx->smc.cxs = &nx->cxs;

	nx->smc.send = nx->rmc.sent = nx->send = msgq_init(send_size);

	nx->rmc.out = nx->recvd = darr_init(recv_size);

	nx->rmc.unpacker = unpacker;
	nx->smc.packer = packer;

	return nx;
}

void
net_receive(struct net_ctx *nx)
{
	recv_msgs(&nx->rmc);
}

void
net_respond(struct net_ctx *nx)
{
	send_msgs(&nx->smc);
}
