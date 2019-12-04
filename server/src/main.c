#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 201900L

#include "sim/action.h"
#include "handle_msg.h"
#include "util/log.h"
#include "sim/world.h"
#include "opts.h"
#include "traps.h"

#include "sim/sim.h"
#include "net/receive.h"
#include "net/respond.h"
#include <time.h>
#include <signal.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>

#define SIMPS 30

struct {
	pthread_t receive;
	pthread_t respond;
	pthread_t handle;
} threads;

void run_simulation(struct simulation *sim)
{
	struct timespec tick = { 0, 1000000000 / SIMPS };

	populate(sim);

	while (1) {
		simulate(sim);
		nanosleep(&tick, NULL);
	}
}

int main(int argc, const char **argv)
{
	process_opts(argc, argv);
	trap_sigint();

	struct world *w = world_init();
	struct server *s = server_init();

	struct simulation *sim = sim_init(w);

	sim->inbound = s->inbound = queue_init();
	sim->outbound  = s->outbound = queue_init();

	pthread_create(&threads.receive, NULL, (void*)net_receive, (void*)s);
	pthread_create(&threads.respond, NULL, (void*)net_respond, (void*)s);
	pthread_create(&threads.handle, NULL, (void*)handle_msgs, (void*)sim);

	run_simulation(sim);

	return 0;
}
