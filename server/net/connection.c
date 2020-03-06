#include <arpa/inet.h>
#include <string.h>

#include "server/net/connection.h"
#include "server/net/server.h"
#include "shared/util/log.h"

void
cx_inspect(const struct connection *c)
{
	struct in_addr addr;

	addr.s_addr = c->addr.ia.sin_addr.s_addr;

	L(
		"client@%p (motiv %d): %s:%d | age: %u",
		c,
		c->motivator,
		inet_ntoa(addr),
		ntohs(c->addr.ia.sin_port),
		c->stale
		);
}

void
cx_init(struct connection *c, const struct sockaddr_in *addr)
{
	memset(c, 0, sizeof(struct connection));
	memcpy(&c->addr, addr, sizeof(struct sockaddr_in));
	c->stale = 0;
	c->motivator = 0;
	c->new = true;
}
