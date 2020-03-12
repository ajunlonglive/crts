#include "server/net.h"
#include "shared/constants/port.h"
#include "shared/messaging/server_message.h"
#include "shared/net/net_ctx.h"
#include "shared/serialize/client_message.h"
#include "shared/serialize/server_message.h"
#include "shared/util/log.h"

static size_t
rmc_unpacker(void *_wm, struct connection *cx, const char *buf)
{
	struct wrapped_message *wm = _wm;

	wm->cx = cx;

	return unpack_cm(&wm->cm, buf);
}

struct net_ctx *
net_init(void)
{
	return net_ctx_init(PORT, INADDR_ANY, sizeof(struct server_message),
		sizeof(struct wrapped_message), rmc_unpacker, pack_sm);
}

void
send_msg(struct net_ctx *nx, enum server_message_type t, const void *dat)
{
	sm_init(msgq_add(nx->send, 0), t, dat);
}
