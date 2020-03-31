#ifndef CRTS_GLOBALS_H
#define CRTS_GLOBALS_H

#include "shared/constants/blueprints.h"
#include "shared/sim/action.h"
#include "shared/sim/ent.h"

enum trav_type {
	trav_no      = 0,
	trav_aquatic = 1 << 0,
	trav_land    = 1 << 1,
};

#define SPAWNABLE_ENTS_LEN 2

struct global_cfg_t {
	const struct {
		const char *name;
		const uint16_t max_workers;
		const uint16_t min_workers;
		const uint16_t completed_at;
		const uint16_t satisfaction;
	} actions[action_type_count];

	const struct {
		const char *name;
		const bool animate;
		const bool holdable;
		const bool rideable;
		const uint16_t hp;
		const uint16_t lifespan;
		const uint16_t spawn_chance;
		const uint16_t group_size;
		enum trav_type trav;
		enum ent_type corpse;
		enum tile spawn_tile;
	} ents[ent_type_count];

	const struct {
		const char *name;
		const uint8_t trav_type;
		const bool functional;
		const bool foundation;
		const bool flamable;
		const uint16_t hardness;
		enum ent_type drop;
		enum tile base;
		enum tile next_to;
		enum tile next;
		enum ent_type makeup;
	} tiles[tile_count];

	const struct {
		const enum ent_type spawnable_ents[SPAWNABLE_ENTS_LEN];
		const uint16_t shrine_spawn_rate;
		const uint16_t shrine_range;
		const uint16_t farm_grow_rate;
		const uint16_t fire_spread_rate;
		const uint16_t fire_spread_chance;
		const uint16_t fire_spread_ignite_chance;
		const uint16_t initial_spawn_range;
		const uint16_t initial_spawn_amount;
		const uint16_t fire_damage;
		const uint16_t meander_chance;
		const uint16_t max_over_age;
		const uint16_t terrain_base_adj_grow_chance;
		const uint16_t terrain_base_not_adj_grow_chance;
		const uint16_t terrain_initial_age_multiplier;
		const uint16_t terrain_initial_age_max;
	} misc;
};

extern const struct global_cfg_t gcfg;
#endif
