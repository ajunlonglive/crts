#ifndef CRTS_GLOBALS_H
#define CRTS_GLOBALS_H

#include "shared/sim/action.h"
#include "shared/sim/ent.h"
#include "shared/pathfind/trav.h"

#define SPAWNABLE_ENTS_LEN 2

struct global_cfg_t {
	const struct {
		const char *name;
		const bool animate;
		const bool holdable;
		const bool phantom;
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
		const bool flamable;
		const bool function;
		const uint8_t hardness;
		enum tile base;
		enum tile next_to;
		enum tile next;
	} tiles[tile_count];

	const struct {
		const enum ent_type spawnable_ents[SPAWNABLE_ENTS_LEN];
		const uint16_t fire_spread_rate;
		const uint16_t fire_spread_chance;
		const uint16_t fire_spread_ignite_chance;
		const uint16_t initial_spawn_range;
		const uint16_t initial_spawn_amount;
		const uint16_t fire_damage;
		const uint16_t meander_chance;
		const uint16_t max_over_age;
	} misc;
};

extern const struct global_cfg_t gcfg;
#endif
