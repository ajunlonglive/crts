#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 201900L

#include "action.h"
#include "log.h"
#include "net.h"
#include "world.h"
#include "sim.h"
#include <time.h>
#include <pthread.h>
#include <stdlib.h>

void *thread_receive(void *server)
{
	struct server *s = server;

	net_receive(s);

	return NULL;
}

void *thread_respond(void *server)
{
	//struct server *s = server;

	//net_respond(s, NULL);

	return NULL;
}

void world_loop(struct simulation *sim)
{
	struct action *act;

	struct timespec tick = {
		.tv_sec = 1,
		.tv_nsec = 0
	};

	populate(sim);

	while (1) {
		act = queue_pop(sim->inbound);

		if (act != NULL)
			sim_add_act(sim, act);

		simulate(sim);
		nanosleep(&tick, NULL);
	}
}

int main(int argc, const char **argv)
{
	pthread_t receive_thread, respond_thread;

	struct world *w = world_init();
	struct server *s = server_init();

	struct simulation *sim = sim_init(w, atoi(argv[1]));

	sim->inbound = s->inbound;
	sim->outbound = s->outbound;

	pthread_create(&receive_thread, NULL, thread_receive, (void*)s);
	pthread_create(&respond_thread, NULL, thread_respond, (void*)s);

	world_loop(sim);

	return 0;
}
