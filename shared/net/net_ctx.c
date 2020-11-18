#include "posix.h"

#include <stdlib.h>
#include <string.h>

#include "shared/net/bind_sock.h"
#include "shared/net/net_ctx.h"
#include "shared/serialize/message.h"
#include "shared/types/darr.h"
#include "shared/util/log.h"

void
net_ctx_init(struct net_ctx *nx,
	uint32_t port, uint32_t addr, message_handler handler,
	uint16_t id)
{
	struct sockaddr_in ia;

	memset(&ia, 0, socklen);
	ia.sin_family = AF_INET;
	ia.sin_port = htons(port);
	ia.sin_addr.s_addr = htonl(addr);
	nx->sock = bind_sock(&ia);

	cx_pool_init(&nx->cxs);

	msgq_init(&nx->send);

	nx->handler = handler;

	nx->id = id;
}

void
queue_msg(struct net_ctx *nx, enum message_type mt, void *msg, cx_bits_t dest,
	enum msg_flags f)
{
	static bool buf_full = false;

	bool appended = nx->buf.msg.count
			&& nx->buf.msg.mt == mt
			&& nx->buf.dest == dest
			&& nx->buf.f == f
			&& append_msg(&nx->buf.msg, msg);

	if (!appended) {
		if (buf_full) {
			msgq_add(&nx->send, &nx->buf.msg, nx->buf.dest, nx->buf.f);
			memset(&nx->buf.msg, 0, sizeof(struct message));
		} else {
			buf_full = true;
		}

		nx->buf.msg.mt = mt;
		nx->buf.dest = dest;
		nx->buf.f = f;

		bool r = append_msg(&nx->buf.msg, msg);

		if (!r) {
			LOG_W("failed to append message");
			assert(false);
		}
	}
}
