#include <fcntl.h>
#include <stdio.h>
#include <string.h>

#include "shared/constants/port.h"
#include "server/net/server.h"

socklen_t socklen = sizeof(struct sockaddr_in);

struct server *
server_init(void)
{
	struct server *s = malloc(sizeof(struct server));

	union {
		struct sockaddr_in ia;
		struct sockaddr sa;
	} addr;

	memset(s, 0, sizeof(struct server));
	memset(&addr, 0, socklen);

	addr.ia.sin_port = htons(PORT);
	addr.ia.sin_addr.s_addr = htonl(INADDR_ANY);

	s->sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

	if (bind(s->sock, &addr.sa, socklen) == -1) {
		perror("bind");
		exit(1);
	}

	int flags = fcntl(s->sock, F_GETFL);
	fcntl(s->sock, F_SETFL, flags | O_NONBLOCK);

	s->cxs = cx_pool_init();

	return s;
}
