#ifndef SERVER_SIM_SIM_H
#define SERVER_SIM_SIM_H

#include "shared/sim/action.h"
#include "shared/sim/ent_buckets.h"
#include "shared/sim/world.h"

struct simulation {
	struct hash eb;
	struct world *world;
	struct darr terrain_mods;
	struct darr players;
	struct darr ents_sorted;

	size_t seq;
	size_t chunk_date;
	uint32_t tick;
	bool paused;
	float t;
};

enum terrain_mod_type {
	terrain_mod_crater,
	terrain_mod_height,
	terrain_mod_moisten,
};

struct terrain_mod {
	struct point pos;
	enum terrain_mod_type type;
	uint16_t r;
	union {
		float height;
	} mod;
};

struct player {
	struct point cursor, ent_center_of_mass;
	enum action action;
	enum ent_type ent_type;
	uint16_t id, ent_count;
};

struct player *add_new_player(struct simulation *sim, uint16_t id);
struct player *get_player(struct simulation *sim, uint16_t id);
struct player *get_nearest_player(struct simulation *sim, struct point *pos, uint32_t max);
void simulate(struct simulation *sim);
void sim_init(struct world *w, struct simulation *sim);
void sim_reset(struct simulation *sim);
#endif
