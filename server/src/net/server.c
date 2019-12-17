#include <stdio.h>
#include <string.h>

#include "constants/port.h"
#include "server.h"

socklen_t socklen = sizeof(struct sockaddr_in);

struct server *server_init(void)
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
		return NULL;
	}

	s->cxs = cx_pool_init();

	return s;
}
