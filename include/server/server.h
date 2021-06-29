#ifndef SERVER_SERVER_H
#define SERVER_SERVER_H

#include "server/opts.h"
#include "server/sim/sim.h"
#include "shared/msgr/msgr.h"
#include "shared/util/timer.h"

struct server {
	struct world w;
	struct simulation sim;
	struct msgr msgr;

	struct {
		struct timer_avg server_tick;
	} prof;
};

bool init_server(struct server *s, struct world_loader *wl,
	struct server_opts *opts);
void server_tick(struct server *s, uint32_t ticks);
#endif
