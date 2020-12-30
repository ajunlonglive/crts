#ifndef SERVER_API_H
#define SERVER_API_H

#include "server/opts.h"
#include "server/sim/sim.h"
#include "shared/msgr/msgr.h"

struct server {
	struct world w;
	struct simulation sim;
	struct msgr msgr;
};

bool init_server(struct server *s, struct server_opts *so);
void server_tick(struct server *s);
#endif
