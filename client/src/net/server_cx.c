#define _DEFAULT_SOURCE
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "util/log.h"
#include "constants/port.h"
#include "server_cx.h"

socklen_t socklen = sizeof(struct sockaddr_in);

void server_cx_init(struct server_cx *s, const char *ipv4addr)
{
	struct sockaddr_in saddr;

	memset(s, 0, sizeof(struct server_cx));
	memset(&saddr, 0, sizeof(struct sockaddr_in));

	s->server_addr.sin_port = htons(PORT);
	inet_aton(ipv4addr, &s->server_addr.sin_addr);
	s->sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

	L("binding to %s", ipv4addr);

	if (bind(s->sock, (struct sockaddr *)&saddr, socklen) != 0)
		perror("bind");
	else
		L("bound socket");
}

