#ifndef __SIM_H
#define __SIM_H
#include "sim/world.h"
#include "types/queue.h"

struct simulation {
	struct world *world;
	struct queue *inbound;
	struct queue *outbound;
	size_t pcnt;
	size_t pcap;
	struct action *pending;

	size_t seq;
};

void populate(struct simulation *sim);
void simulate(struct simulation *sim);
struct simulation *sim_init(struct world *w);
struct action *sim_add_act(struct simulation *sim, const struct action *act);
#endif
