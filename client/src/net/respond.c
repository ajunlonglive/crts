#define _POSIX_C_SOURCE 201900L

#include <time.h>
#include "respond.h"
#include "util/log.h"
#include "messaging/client_message.h"
#include "serialize/client_message.h"

#define POKES_PER_SECOND 30
#define BUFSIZE 255

static const struct timespec tick_dur = { 0, 1000000000 / POKES_PER_SECOND };

static size_t pack_message(const struct client_message *cm, char *buf)
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
	}

	return b;
}

static struct {
	char *buf[sizeof(struct client_message)];
	size_t b;
} poke;

static void poke_init()
{
	poke.b = pack_message(cm_create(client_message_poke, NULL), (char*)poke.buf);
}

void net_respond(struct server_cx *s)
{
	char buf[BUFSIZE], *sbuf;
	struct client_message *cm;
	size_t b;

	poke_init();

	L("heartbeat starting");

	while (1) {
		if ((cm = queue_pop(s->outbound, 0)) != NULL) {
			L("sending an update, type: %d", cm->type);

			b = pack_message(cm, buf);
			sbuf = buf;
		} else {
			b = poke.b;
			sbuf = (char*)poke.buf;
		}

		sendto(s->sock, sbuf, b, 0, (struct sockaddr *)&s->server_addr, socklen);

		nanosleep(&tick_dur, NULL);
	}
}
