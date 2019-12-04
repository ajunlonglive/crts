#include <string.h>
#include <arpa/inet.h>

#include "util/log.h"
#include "constants/port.h"
#include "server_cx.h"

socklen_t socklen = sizeof(struct sockaddr_in);

void server_cx_init(struct server_cx *s, const char *ipv4addr)
{
	struct sockaddr_in saddr;

	memset(s, 0, sizeof(struct server_cx));

	s->server_addr.sin_port = htons(PORT);
	inet_aton(ipv4addr, &s->server_addr.sin_addr);
	s->sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

	if (bind(s->sock, (struct sockaddr *)&saddr, socklen) != 0)
		perror("bind");
	else
		L("bound socket");
}

