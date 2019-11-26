#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 201900L

#include "action.h"
#include "log.h"
#include "net.h"
#include "world.h"
#include "sim.h"
#include <time.h>
#include <signal.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>

struct state {
	struct {
		pthread_t receive;
		pthread_t respond;
	} threads;
};

struct state gs;

void world_loop(struct simulation *sim)
{
	struct action *act;

	struct timespec tick = {
		.tv_sec = 0,
		//       =1000000000
		.tv_nsec = 1000000000 / 30
	};

	populate(sim);

	while (1) {
		act = NULL; //queue_pop(sim->inbound);

		if (act != NULL)
			sim_add_act(sim, act);

		simulate(sim);
		nanosleep(&tick, NULL);
	}
}

static void handle_sigint(_)
{
	L("shutting down");

	pthread_cancel(gs.threads.receive);
	pthread_join(gs.threads.receive, NULL);
	pthread_cancel(gs.threads.respond);
	pthread_join(gs.threads.respond, NULL);

	exit(0);
}

static void install_signal_handler()
{
	struct sigaction sigact;

	memset(&sigact, 0, sizeof(struct sigaction));

	sigact.sa_flags = 0;
	sigact.sa_handler = handle_sigint;
	sigaction(SIGINT, &sigact, NULL);
}

int main(int argc, const char **argv)
{
	int seed;

	if (argc < 2) {
		L("error: please provide a seed");
		return 1;
	} else {
		seed = atoi(argv[1]);
		srandom(seed);
	}

	install_signal_handler();

	struct world *w = world_init();
	struct server *s = server_init();

	struct simulation *sim = sim_init(w);

	sim->inbound = s->inbound;
	sim->outbound = s->outbound;

	pthread_create(&gs.threads.receive, NULL, (void*)net_receive, (void*)s);
	pthread_create(&gs.threads.respond, NULL, (void*)net_respond, (void*)s);

	world_loop(sim);

	return 0;
}
