#include "action.h"
#include "log.h"
#include "net.h"
#include "world.h"
#ifndef __USE_POSIX199309
#define __USE_POSIX199309
#endif
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

void world_loop(struct queue *aq)
{
	struct action *act;
	struct timespec tick = {
		.tv_sec = 0,
		.tv_nsec = 1000
	};

	while (1) {
		act = queue_pop(aq);
		if (act != NULL) {
			L("got an action yo!");
			L("action: { type: %d }", act->type);
		}
		nanosleep(&tick, NULL);
	}
}

int main(int argc, const char **argv)
{
	pthread_t receive_thread, respond_thread;

	struct world *w = world_init();
	struct ent *e = ent_init(5, 5, '@');

	world_spawn(w, e);

	struct server *s = server_init();
	pthread_create(&receive_thread, NULL, thread_receive, (void*)s);
	pthread_create(&respond_thread, NULL, thread_respond, (void*)s);

	world_loop(s->inbound);

	return 0;
}
