#ifndef __SIM_H
#define __SIM_H
#include "world.h"
#include "queue.h"

struct simulation {
	struct world *world;
	struct queue *inbound;
	size_t pcnt;
	size_t pcap;
	struct action *pending;
};

void populate(struct world *w);
void simulate(struct world *sim);
struct simulation *sim_init(struct world *w, struct queue *ib);
void sim_add_act(struct simulation *sim, struct action *act);
#endif
