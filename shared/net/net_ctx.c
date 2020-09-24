#include "posix.h"

#include <stdlib.h>
#include <string.h>

#include "shared/net/bind_sock.h"
#include "shared/net/net_ctx.h"
#include "shared/serialize/message.h"
#include "shared/types/darr.h"
#include "shared/util/log.h"

struct net_ctx *
net_ctx_init(uint32_t port, uint32_t addr, message_handler handler)
{
	struct net_ctx *nx = calloc(1, sizeof(struct net_ctx));
	struct sockaddr_in ia;

	memset(&ia, 0, socklen);
	ia.sin_family = AF_INET;
	ia.sin_port = htons(port);
	ia.sin_addr.s_addr = htonl(addr);
	nx->sock = bind_sock(&ia);

	cx_pool_init(&nx->cxs);

	nx->send = msgq_init();

	nx->handler = handler;

	return nx;
}

void
queue_msg(struct net_ctx *nx, enum message_type mt, void *msg, cx_bits_t dest,
	enum msg_flags f)
{
	if (nx->buf.msg.count) {
		if (nx->buf.msg.mt == mt && nx->buf.dest == dest
		    && nx->buf.f == f) {
			if (!append_msg(&nx->buf.msg, msg)) {
				msgq_add(nx->send, &nx->buf.msg, dest, f);
				memset(&nx->buf.msg, 0, sizeof(struct message));
			}
		} else {
			msgq_add(nx->send, &nx->buf.msg, dest, f);
			memset(&nx->buf.msg, 0, sizeof(struct message));
		}
	} else {
		nx->buf.msg.mt = mt;
		nx->buf.dest = dest;
		nx->buf.f = f;
		if (!append_msg(&nx->buf.msg, msg)) {
			msgq_add(nx->send, &nx->buf.msg, dest, f);
			memset(&nx->buf.msg, 0, sizeof(struct message));
		}
	}
}
