#ifndef CRTS_GLOBALS_H
#define CRTS_GLOBALS_H

#include "shared/sim/action.h"
#include "shared/sim/ent.h"

struct blueprint_block {
	struct point p;
	enum tile t;
};

struct blueprint {
	const struct blueprint_block* blocks;
	struct rectangle lot;
	size_t len;
	uint16_t cost;
};

enum building {
	bldg_block,
	bldg_house,
	bldg_star,
	bldg_tri,
	buildings_count,
};

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
	} ents[ent_type_count];

	const struct blueprint blueprints[buildings_count];

	const struct {
		const uint8_t diff;
		enum ent_type drop;
		enum tile base;
	} harvestable[action_harvest_targets_count];
};

extern const struct global_cfg_t gcfg;
#endif
