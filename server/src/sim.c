#include <string.h>
#include "action.h"
#include "world.h"
#include "queue.h"
#include "sim.h"
#include "log.h"

#define STEP 10

void populate(struct world *w)
{
	size_t i;
	struct ent *e;

	for (i = 0; i < 100; i++) {
		e = world_spawn(w);
		e->x = e->y = i;
	}
}

struct simulation *sim_init(struct world *w, struct queue *ib)
{

	struct simulation *sim = malloc(sizeof(struct simulation));

	sim->world = w;
	sim->inbound = ib;

	return sim;
}

void simulate(struct world *w)
{
	struct ent *e;
	size_t i;

	for (i = 0; i < w->ecnt; i++) {
		e = &w->ents[i];

		e->age++;
		if (e->satisfaction > 0)
			e->satisfaction--;
	}
}

void sim_add_act(struct simulation *sim, struct action *act)
{
	if (sim->pcnt + 1 >= sim->pcap) {
		sim->pcap += STEP;
		sim->pending = realloc(sim->pending, sizeof(struct action) * sim->pcap);
	}

	memcpy(&sim->pending[sim->pcnt - 1], act, sizeof(struct action));
	sim->pcnt++;
}
