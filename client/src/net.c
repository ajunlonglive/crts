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

#define BUFSIZE 255

static socklen_t socklen = sizeof(struct sockaddr_in);

struct server *net_connect()
{
	struct server *s = malloc(sizeof(struct server));

	memset(s, 0, sizeof(struct server));
	s->server_addr.sin_port = htons(PORT);
	inet_aton("127.0.0.1", &s->server_addr.sin_addr);
	s->sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (bind(s->sock, (struct sockaddr *)&s->listen_addr, socklen) != 0)
		perror("bind");
	L("successfully bound socket");

	s->inbound = queue_init();

	return s;
}

void net_respond(struct server *s)
{
	char buf = 0;
	int res;
	struct timespec tick = {
		.tv_sec = 0,
		//v_nsec = 999999999
		.tv_nsec =  33333333
	};

	L("heartbeat starting");

	while (1) {
		res = sendto(s->sock, &buf, 1, 0, (struct sockaddr *)&s->server_addr, socklen);

		nanosleep(&tick, NULL);
	}
}

void net_receive(struct server *s)
{
	char buf[BUFSIZE];
	int res;

	struct sockaddr_in saddr;
	struct update *ud;

	struct pollfd pfd = {
		.fd = s->sock,
		.events = POLLIN,
		.revents = 0
	};

	L("listening");
	while (1) {
		poll(&pfd, 1, -1);

		res = recvfrom(s->sock, buf, BUFSIZE, 0, (struct sockaddr *)&saddr, &socklen);
		if (res > 0)
			L("received %s (%d) from %s:%d", buf, res, inet_ntoa(saddr.sin_addr), ntohs(saddr.sin_port));

		ud = ent_update_init(NULL);
		unpack_update(ud, buf);
		queue_push(s->inbound, ud);
	}
}
