#ifndef SERVER_SIM_SIM_H
#define SERVER_SIM_SIM_H

#include "server/sim/ent_buckets.h"
#include "server/sim/pathfind/pgraph.h"
#include "shared/sim/world.h"

struct simulation {
	struct ent_buckets eb;
	struct world *world;
	struct hdarr *actions;
	struct hash *deleted_actions;

	size_t seq;
	size_t chunk_date;
	uint32_t tick;
};

uint16_t add_new_motivator(struct simulation *sim);
void simulate(struct simulation *sim);
struct simulation *sim_init(struct world *w);
void harvest_tile(struct world *w, struct point *p, uint16_t mot, uint32_t tick);
#endif
