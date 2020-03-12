#include <arpa/inet.h>
#include <string.h>

#include "shared/net/connection.h"
#include "shared/util/log.h"

void
cx_inspect(const struct connection *c)
{
	struct in_addr addr;

	addr.s_addr = c->addr.ia.sin_addr.s_addr;

	L("cx@%p %s:%d | bit: %x age: %u",
		c,
		inet_ntoa(addr),
		ntohs(c->addr.ia.sin_port),
		c->bit,
		c->stale
		);
}

void
cx_init(struct connection *c, const struct sockaddr_in *addr)
{
	memset(c, 0, sizeof(struct connection));
	memcpy(&c->addr, addr, sizeof(struct sockaddr_in));
	c->new = true;
}
