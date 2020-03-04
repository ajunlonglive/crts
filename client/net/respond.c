#define _POSIX_C_SOURCE 201900L

#include <time.h>

#include "client/net/respond.h"
#include "shared/messaging/client_message.h"
#include "shared/serialize/client_message.h"
#include "shared/util/log.h"

#define POKES_PER_SECOND 30
#define BUFSIZE 255

static size_t
pack_message(const struct client_message *cm, char *buf)
{
	size_t b;

	b = pack_cm(cm, buf);

	switch (cm->type) {
	case client_message_poke:
		break;
	case client_message_action:
		b += pack_cm_action(cm->update, &buf[b]);
		break;
	case client_message_chunk_req:
		b += pack_cm_chunk_req(cm->update, &buf[b]);
		break;
	case client_message_ent_req:
		b += pack_cm_ent_req(cm->update, &buf[b]);
		break;
	}

	return b;
}

static struct {
	char *buf[sizeof(struct client_message)];
	size_t b;
} poke;

static void
poke_init(void)
{
	poke.b = pack_message(cm_create(client_message_poke, NULL), (char*)poke.buf);
}

void
net_respond_init(void)
{
	poke_init();
}

void
net_respond(struct server_cx *s)
{
	char buf[BUFSIZE], *sbuf;
	struct client_message *cm;
	size_t b;

	do {
		if ((cm = queue_pop(s->outbound)) != NULL) {
			b = pack_message(cm, buf);
			sbuf = buf;
		} else {
			b = poke.b;
			sbuf = (char*)poke.buf;
		}

		sendto(s->sock, sbuf, b, 0, &s->server_addr.sa, socklen);
	} while (cm != NULL);
}
