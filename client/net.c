#include <string.h>

#include "client/net.h"
#include "shared/constants/port.h"
#include "shared/net/net_ctx.h"
#include "shared/net/pool.h"
#include "shared/serialize/client_message.h"
#include "shared/serialize/server_message.h"

static long outbound_id;

static size_t
rmc_unpacker(void *_sm, struct connection *_, const char *buf)
{
	struct server_message *sm = _sm;
	return unpack_sm(sm, buf);
}

struct net_ctx *
net_init(const char *ipv4addr)
{
	struct sockaddr_in ia;
	struct net_ctx *nx;

	nx = net_ctx_init(0, 0, sizeof(struct client_message),
		sizeof(struct server_message), rmc_unpacker, pack_cm);

	memset(&ia, 0, sizeof(struct sockaddr_in));
	ia.sin_family = AF_INET;
	ia.sin_port = htons(PORT);
	inet_aton(ipv4addr, &ia.sin_addr);
	cx_establish(&nx->cxs, &ia);

	return nx;
}

void
send_msg(struct net_ctx *nx, enum client_message_type t, const void *dat,
	enum msg_flags f)
{
	struct client_message *cm;

	if ((cm = msgq_add(nx->send, nx->cxs.cx_bits, 0)) != NULL) {
		cm_init(cm, t, dat);
		cm->client_id = outbound_id;
	}
}

void
net_set_outbound_id(long id)
{
	outbound_id = id;
}
