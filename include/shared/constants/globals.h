#ifndef CRTS_GLOBALS_H
#define CRTS_GLOBALS_H

#include "shared/constants/blueprints.h"
#include "shared/sim/action.h"
#include "shared/sim/ent.h"

#define SPAWNABLE_ENTS_LEN 1

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
		uint16_t hp;
		uint16_t lifespan;
		uint16_t spawn_chance;
		uint16_t group_size;
		enum ent_type corpse;
		enum tile spawn_tile;
	} ents[ent_type_count];

	const struct {
		const char *name;
		bool traversable;
		bool functional;
		bool foundation;
		bool flamable;
		uint16_t hardness;
		enum ent_type drop;
		enum tile base;
		enum tile next_to;
		enum tile next;
		enum ent_type makeup;
	} tiles[tile_count];

	struct {
		enum ent_type spawnable_ents[SPAWNABLE_ENTS_LEN];
		uint16_t shrine_spawn_rate;
		uint16_t shrine_range;
		uint16_t farm_grow_rate;
		uint16_t fire_spread_rate;
		uint16_t fire_spread_chance;
		uint16_t fire_spread_ignite_chance;
		uint16_t initial_spawn_range;
		uint16_t initial_spawn_amount;
		uint16_t fire_damage;
		uint16_t meander_chance;
		uint16_t max_over_age;
		uint16_t terrain_base_adj_grow_chance;
		uint16_t terrain_base_not_adj_grow_chance;
		uint16_t terrain_initial_age_multiplier;
		uint16_t terrain_initial_age_max;
	} misc;
};

extern const struct global_cfg_t gcfg;
#endif
