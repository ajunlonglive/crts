#include "posix.h"

#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "server/aggregate_msgs.h"
#include "server/handle_msg.h"
#include "server/net.h"
#include "server/opts.h"
#include "server/sim/sim.h"
#include "shared/net/net_ctx.h"
#include "shared/sim/action.h"
#include "shared/sim/world.h"
#include "shared/util/log.h"
#include "shared/util/time.h"

#define TICK NS_IN_S / 30

int
main(int argc, const char **argv)
{
	logfile = stderr;

	process_opts(argc, argv);

	struct world *w = world_init();
	struct net_ctx *nx = net_init();

	struct simulation *sim = sim_init(w);
	struct timespec tick_st;
	long slept_ns = 0;

	handle_msgs_init();

	clock_gettime(CLOCK_REALTIME, &tick_st);

	while (1) {
		net_receive(nx);

		handle_msgs(sim, nx);

		simulate(sim);

		aggregate_msgs(sim, nx);

		net_respond(nx);

		slept_ns = sleep_remaining(&tick_st, TICK, slept_ns);
	}

	return 0;
}
