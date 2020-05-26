#include "posix.h"

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "shared/net/defs.h"
#include "shared/util/log.h"

int
bind_sock(struct sockaddr_in *ia)
{
	int sock, flags;
	union {
		struct sockaddr sa;
		struct sockaddr_in ia;
	} saddr = { .ia = *ia };

	sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

	if (bind(sock, &saddr.sa, socklen) != 0) {
		perror("bind");
		exit(1);
	}

	L("bound socket");

	flags = fcntl(sock, F_GETFL);
	fcntl(sock, F_SETFL, flags | O_NONBLOCK);

	return sock;
}
