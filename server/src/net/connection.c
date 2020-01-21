#include <string.h>
#include <arpa/inet.h>

#include "util/log.h"
#include "server.h"
#include "connection.h"

void
cx_inspect(const struct connection *c)
{
	struct in_addr addr;

	addr.s_addr = c->addr;

	L(
		"client@%p (motiv %d): %s:%d | age: %ld",
		c,
		c->motivator,
		inet_ntoa(addr),
		ntohs(c->port),
		c->stale
		);
}

void
cx_init(struct connection *c, const struct sockaddr_in *addr)
{
	c->addr = addr->sin_addr.s_addr;
	c->port = addr->sin_port;
	memset(&c->saddr, 0, sizeof(struct sockaddr_in));
	c->stale = 0;
	c->motivator = 0;
}
