#define _DEFAULT_SOURCE

#include <stdlib.h>
#include <pthread.h>
#include <locale.h>

#include "util/log.h"
#include "display.h"
#include "net/receive.h"
#include "net/respond.h"
#include "sim.h"
#include "sim/world.h"
#include "types/queue.h"
#include "world_update.h"

struct {
	pthread_t receive;
	pthread_t respond;
	pthread_t update;
} threads;

char default_addr[] = "127.0.0.1";

int main(int argc, const char **argv)
{
	setlocale(LC_ALL, "");

	struct simulation sim = {
		.w = world_init(),
		.inbound = queue_init(),
		.outbound = queue_init(),
		.run = 1,
		.actions = {
			.e = NULL,
			.len = 0,
			.cap = 0
		}
	};
	struct server_cx scx;

	// connect to server
	server_cx_init(&scx, argc < 2 ? default_addr : argv[1]);
	scx.inbound  = sim.inbound;
	scx.outbound = sim.outbound;

	pthread_create(&threads.receive, NULL, (void*)(*net_receive), &scx);
	pthread_create(&threads.respond, NULL, (void*)(*net_respond), &scx);
	pthread_create(&threads.update, NULL, (void*)(*world_update), &sim);

	display(&sim);

	L("shutting down");
	return 0;
}
