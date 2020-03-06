#define _XOPEN_SOURCE 500

#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "server/handle_msg.h"
#include "server/net/receive.h"
#include "server/net/respond.h"
#include "server/opts.h"
#include "server/sim/sim.h"
#include "server/traps.h"
#include "shared/sim/action.h"
#include "shared/sim/world.h"
#include "shared/util/log.h"
#include "shared/util/time.h"

#define TICK NS_IN_S / 30

int
main(int argc, const char **argv)
{
	process_opts(argc, argv);
	trap_sigint();

	struct world *w = world_init();
	struct server *s = server_init();

	struct simulation *sim = sim_init(w);
	struct timespec tick_st;
	long slept_ns = 0;

	sim->inbound = s->inbound = queue_init();
	sim->outbound  = s->outbound = queue_init();

	net_receive_init();
	handle_msgs_init();

	clock_gettime(CLOCK_REALTIME, &tick_st);

	while (1) {
		net_receive(s);

		handle_msgs(sim);

		simulate(sim);

		net_respond(s);

		slept_ns = sleep_remaining(&tick_st, TICK, slept_ns);
	}

	return 0;
}
