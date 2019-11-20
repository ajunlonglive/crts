#define _DEFAULT_SOURCE
#include "log.h"
#include "net.h"
#include "port.h"
#include "world.h"
#include <arpa/inet.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static struct world *w;

void net_demo()
{
	struct sockaddr_in *server;
	struct in_addr addr;
	int res;

	struct ent e;

	w = world_init();

	server = malloc(sizeof(struct sockaddr_in));
	memset(server, 0, sizeof(struct sockaddr_in));
	L("memset to 0");
	server->sin_port = htons(PORT);
	L("set sin_port to %d", ntohs(server->sin_port));

	inet_aton("127.0.0.1", &addr);
	server->sin_addr = addr;
	L("set server addr");

	int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	L("opened sock %d", sock);
	socklen_t socklen = sizeof(struct sockaddr_in);

	res = sendto(sock, "test", 4, 0, (struct sockaddr *)server, socklen);
	L("sent %d bytes", res);
	char r;
	res = recvfrom(sock, &r, 1, 0, (struct sockaddr *)server, &socklen);
	L("received %c (%d)", r, res);
}
