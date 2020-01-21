#define _DEFAULT_SOURCE
#include <string.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "util/log.h"
#include "constants/port.h"
#include "server_cx.h"

socklen_t socklen = sizeof(struct sockaddr_in);

void
server_cx_init(struct server_cx *s, const char *ipv4addr)
{
	union {
		struct sockaddr_in ia;
		struct sockaddr sa;
	} saddr;

	memset(s, 0, sizeof(struct server_cx));
	memset(&saddr, 0, socklen);

	s->server_addr.ia.sin_port = htons(PORT);
	inet_aton(ipv4addr, &s->server_addr.ia.sin_addr);
	s->sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

	L("binding to %s", ipv4addr);

	if (bind(s->sock, &saddr.sa, socklen) != 0) {
		perror("bind");
		exit(1);
	} else {
		L("bound socket");

		int flags = fcntl(s->sock, F_GETFL);
		fcntl(s->sock, F_SETFL, flags | O_NONBLOCK);
	}
}
