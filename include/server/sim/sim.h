#ifndef SERVER_SIM_SIM_H
#define SERVER_SIM_SIM_H

#include "server/sim/pathfind/pgraph.h"
#include "shared/sim/world.h"

struct simulation {
	struct world *world;
	struct {
		struct sim_action *e;
		size_t len;
		size_t cap;
	} actions;

	size_t seq;
	size_t chunk_date;
	uint32_t tick;
};

void kill_ent(struct simulation *sim, struct ent *e);
struct ent *spawn_ent(struct simulation *sim);

void destroy_tile(struct simulation *sim, struct point *p);

uint16_t add_new_motivator(struct simulation *sim);
void simulate(struct simulation *sim);
struct simulation *sim_init(struct world *w);
enum result pathfind_and_update(struct simulation *sim, struct pgraph *pg, struct ent *e);
void drop_held_ent(struct simulation *sim, struct ent *e);
#endif
