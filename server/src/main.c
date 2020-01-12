#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 201900L

#include "sim/action.h"
#include "handle_msg.h"
#include "util/log.h"
#include "util/time.h"
#include "sim/world.h"
#include "opts.h"
#include "traps.h"

#include "sim/sim.h"
#include "net/receive.h"
#include "net/respond.h"
#include <time.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>

#define TICK NS_IN_S / 30

int main(int argc, const char **argv)
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

	populate(sim);

	net_receive_init();

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
