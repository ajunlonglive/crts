#define _DEFAULT_SOURCE
#include "util/log.h"
#include "receive.h"
#include "serialize/server_message.h"
#include "messaging/server_message.h"
#include <string.h>
#include <poll.h>

#define BUFSIZE 255
#define HEAP_SIZE 255

struct message_heap {
	struct {
		struct server_message e[HEAP_SIZE];
		size_t i;
	} sm;

	struct {
		struct sm_ent e[HEAP_SIZE];
		size_t i;
	} ent;

	struct {
		struct sm_chunk e[HEAP_SIZE];
		size_t i;
	} chunk;

};

static void wrap_inc(size_t *i)
{
	*i = *i >= HEAP_SIZE - 1 ? 0 : *i + 1;
}

static struct server_message *unpack_message(struct message_heap *mh, const char *buf)
{
	size_t b;
	struct server_message *sm;

	sm = &mh->sm.e[mh->sm.i];
	wrap_inc(&mh->sm.i);

	b = unpack_sm(sm, buf);

	switch (sm->type) {
	case server_message_ent:
		b += unpack_sm_ent(&mh->ent.e[mh->ent.i], &buf[b]);

		wrap_inc(&mh->ent.i);
		break;
	case server_message_chunk:
		b += unpack_sm_chunk(&mh->chunk.e[mh->chunk.i], &buf[b]);

		wrap_inc(&mh->chunk.i);
		break;
	}

	return sm;
}

void net_receive(struct server_cx *s)
{
	char buf[BUFSIZE];
	int b;
	struct message_heap *mh;
	struct pollfd pfd = { s->sock, POLLIN, 0 };
	struct server_message *sm;
	struct sockaddr_in saddr;

	mh = malloc(sizeof(struct message_heap));
	memset(mh, 0, sizeof(struct message_heap));

	L("listening to %s:%d", inet_ntoa(s->server_addr.sin_addr), ntohs(s->server_addr.sin_port));

	while (1) {
		poll(&pfd, 1, -1);

		b = recvfrom(s->sock, buf, BUFSIZE, 0, (struct sockaddr *)&saddr, &socklen);

		if (b < 1)
			continue;

		sm = unpack_message(mh, buf);

		queue_push(s->inbound, sm);
	}
}
