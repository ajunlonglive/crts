#include <stdio.h>
#include <string.h>

#include "constants/port.h"
#include "server.h"

struct server *server_init()
{
	struct server *s = malloc(sizeof(struct server));
	struct sockaddr_in addr;

	memset(s, 0, sizeof(struct server));

	addr.sin_port = htons(PORT);
	addr.sin_addr.s_addr = htonl(INADDR_ANY);

	s->sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

	if (bind(s->sock, (struct sockaddr *)&addr, socklen) == -1) {
		perror("bind");
		return NULL;
	}

	return s;
}
