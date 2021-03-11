#ifndef SERVER_SIM_SIM_H
#define SERVER_SIM_SIM_H

#include "shared/sim/action.h"
#include "shared/sim/ent_buckets.h"
#include "shared/sim/world.h"

struct simulation {
	struct ent_buckets eb;
	struct world *world;
	struct hdarr actions;
	struct hash deleted_actions;
	struct darr players;

	size_t seq;
	size_t chunk_date;
	uint32_t tick;
};

struct player {
	struct point cursor;
	enum action action;
	uint16_t id;
};

struct player *add_new_player(struct simulation *sim, uint16_t id);
void simulate(struct simulation *sim);
void sim_init(struct world *w, struct simulation *sim);
void harvest_tile(struct world *w, struct point *p, uint16_t mot, uint32_t tick);
#endif
