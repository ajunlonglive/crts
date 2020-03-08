#ifndef CRTS_SERVER
#define CRTS_SERVER
#endif

#include "server/net/respond.h"
#include "shared/serialize/server_message.h"
#include "shared/types/queue.h"
#include "shared/util/log.h"

#define BUFSIZE 4096

static size_t
pack_buffer(char *buf, struct server_message *sm)
{
	size_t b;

	b = pack_sm(sm, buf);

	switch (sm->type) {
	case server_message_ent:
		b += pack_sm_ent(sm->update, &buf[b]);
		break;
	case server_message_chunk:
		b += pack_sm_chunk(sm->update, &buf[b]);
		break;
	case server_message_action:
		b += pack_sm_action(sm->update, &buf[b]);
		break;
	case server_message_rem_action:
		b += pack_sm_rem_action(sm->update, &buf[b]);
		break;
	case server_message_world_info:
		b += pack_sm_world_info(sm->update, &buf[b]);
		break;
	case server_message_hello:
		b += pack_sm_hello(sm->update, &buf[b]);
		break;
	case server_message_kill_ent:
		b += pack_sm_kill_ent(sm->update, &buf[b]);
		break;
	}

	return b;
}


struct msg_ctx {
	int sock;
	char *buf;
	size_t b;
};

static enum iteration_result
send_message(void *_ctx, void *_cx)
{
	struct connection *cx = _cx;
	struct msg_ctx *ctx = _ctx;

	sendto(ctx->sock, ctx->buf, ctx->b, 0, &cx->addr.sa, socklen);

	return ir_cont;
}

void
net_respond(struct server *s)
{
	char buf[BUFSIZE] = "";
	size_t b;
	struct server_message *sm = NULL;

	while ((sm = queue_pop(s->outbound)) != NULL) {
		b = pack_buffer(buf, sm);

		struct msg_ctx ctx = {
			.sock = s->sock,
			.buf = buf,
			.b = b
		};

		if (sm->dest != NULL) {
			send_message(&ctx, sm->dest);
		} else {
			hdarr_for_each(s->cxs->cxs, &ctx, send_message);
		}

		sm_destroy(sm);
	}
}
