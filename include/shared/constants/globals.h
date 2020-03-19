#ifndef CRTS_GLOBALS_H
#define CRTS_GLOBALS_H

#include "shared/constants/blueprints.h"
#include "shared/sim/action.h"
#include "shared/sim/ent.h"

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
		uint16_t lifespan;
	} ents[ent_type_count];

	const struct {
		const uint8_t diff;
		enum ent_type drop;
		enum tile tgt;
		enum tile base;
	} harvestable[action_harvest_targets_count];

	const struct {
		const char *name;
		bool traversable;
		bool foundation;
		enum tile next;
		enum ent_type makeup;
	} tiles[tile_count];
};

extern const struct global_cfg_t gcfg;
#endif
