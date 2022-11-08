#ifndef SHARED_SIM_ENT_H
#define SHARED_SIM_ENT_H

#include <stdbool.h>
#include <stdint.h>

#include "shared/pathfind/api.h"
#include "shared/math/geom.h"
#include "shared/sim/world.h"
#include "shared/types/result.h"

enum ent_type {
	et_sand,
	et_fire,
	et_wood,
	et_acid,
	et_water,
	et_spring,
	et_explosion,
	ent_type_count
};

enum ent_states {
	es_killed       = 1 << 0,
	es_spawned      = 1 << 1,
	es_modified     = 1 << 2,
	es_pathfinding  = 1 << 3,
};

enum ent_update_type {
	eu_pos       = 1 << 0,
	ent_update_type_max = (eu_pos) + 1,
};

typedef uint32_t ent_id_t;

struct ent {
	struct point pos;
	int16_t z;
	float velocity[3];
	float real_pos[3];
	ent_id_t id;
	enum ent_type type;
	uint32_t path;
	uint32_t target;
	uint16_t loyalty;
	uint16_t age;
	uint8_t damage;
	uint8_t trav;
	uint8_t state;
	uint8_t modified;
};

void ent_init(struct ent *e);

typedef bool (*find_ent_predicate)(void *ctx, struct ent *e);
struct ent *find_ent(struct world *w, const struct point *p, void *ctx,
	find_ent_predicate epred);
#endif
