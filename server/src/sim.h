#ifndef __SIM_H
#define __SIM_H
#include <stdlib.h>
#include "world.h"
#include "queue.h"

#define STATEBUF_LEN 255

struct simulation {
	struct world *world;
	struct queue *inbound;
	struct queue *outbound;
	size_t pcnt;
	size_t pcap;
	struct action *pending;

	size_t seq;

	char *statebuf;
	struct random_data prng;
};

void populate(struct simulation *sim);
void simulate(struct simulation *sim);
struct simulation *sim_init(struct world *w, int seed);
struct action *sim_add_act(struct simulation *sim, const struct action *act);
#endif
