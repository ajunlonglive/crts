#include "log.h"
#include "net.h"
#include "port.h"
#include "queue.h"
#include "world.h"
#include "serialize.h"
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifndef __USE_POSIX199309
#define __USE_POSIX199309
#endif
#include <time.h>

#define BUFSIZE 255
#define STEP 5
#define STALE_THRESHOLD 1000
static socklen_t socklen = sizeof(struct sockaddr_in);

static void inspect_client(struct client *c)
{
	struct in_addr addr;

	addr.s_addr = c->addr;

	L(
		"client@%p: %s:%d | age: %d",
		c,
		inet_ntoa(addr),
		ntohs(c->port),
		c->stale
		);
}

struct server *server_init()
{
	struct server *s = malloc(sizeof(struct server));

	memset(s, 0, sizeof(struct server));

	s->addr.sin_port = htons(PORT);
	s->addr.sin_addr.s_addr = htonl(INADDR_ANY);

	s->sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

	s->inbound = queue_init();
	s->outbound = queue_init();

	if (bind(s->sock, (struct sockaddr *)&s->addr, socklen) == -1) {
		perror("bind");
		return NULL;
	}

	return s;
}

static void client_init(struct client *c, struct sockaddr_in *addr)
{
	c->addr = addr->sin_addr.s_addr;
	c->port = addr->sin_port;
	c->stale = 0;
}

static void add_client(struct server *s, struct sockaddr_in *addr)
{
	struct client *cl;

	s->cxs.len++;

	if (s->cxs.len >= s->cxs.cap) {
		s->cxs.cap += STEP;
		s->cxs.l = realloc(s->cxs.l, sizeof(struct client) * s->cxs.cap);
		memset(&s->cxs.l[s->cxs.cap - STEP], 0, sizeof(struct client) * STEP);
		L("increased client array capacity to %d", s->cxs.cap);
	}

	cl = &s->cxs.l[s->cxs.len - 1];
	client_init(cl, addr);
	L("new client connected!");
	inspect_client(cl);
}

int find_and_touch_client(struct server *s, struct sockaddr_in *caddr)
{
	size_t i;
	struct client *cl;

	for (i = 0; i < s->cxs.len; i++) {
		cl = &s->cxs.l[i];

		if (cl->addr == caddr->sin_addr.s_addr && cl->port == caddr->sin_port) {
			L("existing connection");
			cl->stale = 0;
			return i;
		}
	}

	return -1;
}

void remove_client(struct server *s, size_t id)
{
	struct client *cl;

	s->cxs.len--;

	if (s->cxs.len == 0) {
		memset(s->cxs.l, 0, sizeof(struct client));
		return;
	}

	cl = &s->cxs.l[s->cxs.len];
	memcpy(cl, &s->cxs.l[id], sizeof(struct client));
	memset(cl, 0, sizeof(struct client));
}

static void age_clients(struct server *s)
{
	size_t i;
	struct client *cl;
	struct in_addr e;

	for (i = 0; i < s->cxs.len; i++) {
		cl = &s->cxs.l[i];

		cl->stale += 1;

		e.s_addr = cl->addr;

		if (cl->stale >= STALE_THRESHOLD) {
			L("removing client");
			inspect_client(cl);
			remove_client(s, i);
		}
	}
}

void net_receive(struct server *s)
{
	struct sockaddr_in caddr;
	struct timespec server_recv_tick = {
		.tv_sec = 0,
		.tv_nsec = 1000
	};
	char buf[BUFSIZE];
	int res;
	size_t acti = 0;
	struct action acts[s->inbound->cap];

	while (1) {
		res = recvfrom(s->sock, buf, BUFSIZE, MSG_DONTWAIT,
			       (struct sockaddr *)&caddr, &socklen);

		if (res > 0) {
			L("got %d bytes", res);
			if (find_and_touch_client(s, &caddr) == -1)
				add_client(s, &caddr);

			unpack_action(&acts[acti], buf);
			L("got action { type: %d }", acts[acti].type);

			queue_push(s->inbound, &acts[acti]);

			acti++;
			if (acti >= s->inbound->cap)
				acti = 0;
		}

		age_clients(s);
		nanosleep(&server_recv_tick, NULL);
	}
}

void net_respond(struct server *s)
{
	size_t i;
	struct client *cl;

	char *msg = "jej";
	size_t msglen = 3;

	while (1) {
		for (i = 0; i < s->cxs.len; i++) {
			cl = &s->cxs.l[i];

			sendto(
				s->sock,
				msg,
				msglen,
				MSG_DONTWAIT,
				(struct sockaddr *)&s->addr,
				socklen
				);
		}
	}
}
