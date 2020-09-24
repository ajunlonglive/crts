#include "posix.h"

#include "server/handle_msg.h"
#include "server/net.h"
#include "shared/constants/port.h"
#include "shared/net/net_ctx.h"
#include "shared/serialize/message.h"
#include "shared/util/log.h"

struct net_ctx *
net_init(struct simulation *sim)
{
	struct net_ctx *nx = net_ctx_init(PORT, INADDR_ANY, handle_msg);
	nx->usr_ctx = sim;
	return nx;
}

void
broadcast_msg(struct net_ctx *nx, enum message_type t, void *dat,
	enum msg_flags f)
{
	queue_msg(nx, t, dat, nx->cxs.cx_bits, f);
}
