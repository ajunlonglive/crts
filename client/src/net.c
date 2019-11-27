#define _DEFAULT_SOURCE
#include "log.h"
#include "net.h"
#include "port.h"
#include "world.h"
#include "serialize.h"
#include <arpa/inet.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <poll.h>

#define TPS 30
#define BUFSIZE 255

static socklen_t socklen = sizeof(struct sockaddr_in);

struct cxinfo *net_connect(const char *ipv4addr)
{
	struct cxinfo *s = malloc(sizeof(struct cxinfo));

	memset(s, 0, sizeof(struct cxinfo));
	s->server_addr.sin_port = htons(PORT);
	inet_aton(ipv4addr, &s->server_addr.sin_addr);
	s->sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (bind(s->sock, (struct sockaddr *)&s->listen_addr, socklen) != 0)
		perror("bind");
	L("successfully bound socket");

	s->inbound = queue_init();
	s->outbound = queue_init();
	s->run = NULL;

	return s;
}

void net_respond(struct cxinfo *s)
{
	char ubuf[BUFSIZE];
	char pbuf[sizeof(int)];
	char *buf;
	size_t pbsize, ubsize, *size;
	int res, reset = 0;
	struct timespec tick = { 0, 1000000000 / TPS };
	struct update *poke = poke_update_init();
	struct update *update;

	pbsize = pack_update(poke, pbuf);
	buf = pbuf;
	size = &pbsize;

	L("heartbeat starting");

	while (s->run != NULL && *s->run) {
		if ((update = queue_pop(s->outbound, 0)) != NULL) {
			ubsize = pack_update(update, ubuf);
			switch (update->type) {
			case update_type_action:
				ubsize += pack_action_update(update->update, &ubuf[ubsize]);
				break;
			default:
				break;
			}

			L("sending an update, type: %d, size: %d", update->type, ubsize);
			buf = ubuf;
			size = &ubsize;
			reset = 1;
		}

		res = sendto(s->sock, buf, *size, 0, (struct sockaddr *)&s->server_addr, socklen);

		nanosleep(&tick, NULL);

		if (reset) {
			buf = pbuf;
			size = &pbsize;
			reset = 0;
		}
	}
}

void net_receive(struct cxinfo *s)
{
	char buf[BUFSIZE];
	int res;

	struct sockaddr_in saddr;
	size_t ui = 0, eui = 0, b;

	struct update *updates =
		calloc(s->inbound->cap, sizeof(struct update));
	struct ent_update *ent_updates =
		calloc(s->inbound->cap, sizeof(struct ent_update));

	struct pollfd pfd = {
		.fd = s->sock,
		.events = POLLIN,
		.revents = 0
	};

	L("listening to %s:%d", inet_ntoa(s->server_addr.sin_addr), ntohs(s->server_addr.sin_port));

	while (1) {
		poll(&pfd, 1, -1);

		res = recvfrom(s->sock, buf, BUFSIZE, 0, (struct sockaddr *)&saddr, &socklen);

		b = unpack_update(&updates[ui], buf);

		switch (updates[ui].type) {
		case update_type_ent:
			unpack_ent_update(&ent_updates[eui], &buf[b]);
			updates[ui].update = &ent_updates[eui];
			eui = eui >= s->inbound->cap - 1 ? 0 : eui + 1;
			break;
		case update_type_poke:
		case update_type_action:
			break;
		}

		queue_push(s->inbound, &updates[ui]);

		ui = ui >= s->inbound->cap - 1 ? 0 : ui + 1;
	}
}
