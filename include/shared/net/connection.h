#ifndef SHARED_NET_CONNECTION_H
#define SHARED_NET_CONNECTION_H

#include <stdbool.h>

#include "shared/net/defs.h"
#include "shared/net/ack.h"

struct connection {
	struct acks acks;

	union {
		struct sockaddr_in ia;
		struct sockaddr sa;
	} addr;

	uint32_t stale;
	msg_ack_t bit;
	uint16_t motivator;
	bool new;
};

void cx_inspect(const struct connection *c);
void cx_init(struct connection *c, const struct sockaddr_in *addr);
#endif
