#ifndef SERVER_SIM_SIM_H
#define SERVER_SIM_SIM_H

#include "shared/sim/action.h"
#include "shared/sim/ent_buckets.h"
#include "shared/sim/world.h"

struct simulation {
	struct ent_buckets eb;
	struct world *world;
	struct hdarr actions;
	struct hdarr terrain_mods;
	struct darr players;

	size_t seq;
	size_t chunk_date;
	uint32_t tick;
	bool paused;
};

struct terrain_mod {
	struct point pos;
	float mod;
};

struct player {
	struct point cursor, ent_center_of_mass;
	enum action action;
	uint16_t id, ent_count;
};

struct player *add_new_player(struct simulation *sim, uint16_t id);
struct player *get_player(struct simulation *sim, uint16_t id);
struct player *get_nearest_player(struct simulation *sim, struct point *pos, uint32_t max);
void simulate(struct simulation *sim);
void sim_init(struct world *w, struct simulation *sim);
#endif
