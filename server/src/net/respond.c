#include "types/queue.h"
#include "respond.h"
#include "util/log.h"
#include "serialize/server_message.h"

#define BUFSIZE 2048

void net_respond(struct server *s)
{
	char buf[BUFSIZE] = "";
	size_t i, b;
	struct connection *cx = NULL;
	struct server_message *sm = NULL;

	while ((sm = queue_pop(s->outbound)) != NULL) {
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
		}

		for (i = 0; i < s->cxs->mem.len; i++) {
			cx = &s->cxs->mem.cxs[i];

			sendto(s->sock, buf, b, MSG_DONTWAIT, &cx->saddr.sa, socklen);
		}

		sm_destroy(sm);
	}
}
