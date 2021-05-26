#ifndef SHARED_SIM_ENT_H
#define SHARED_SIM_ENT_H

#include <stdbool.h>
#include <stdint.h>

#include "shared/pathfind/api.h"
#include "shared/math/geom.h"
#include "shared/sim/world.h"
#include "shared/types/result.h"

enum ent_type {
	et_none,
	et_worker,
	et_elf_corpse,
	et_deer,
	et_fish,
	et_resource_wood,
	et_resource_meat,
	et_resource_rock,
	et_resource_crop,
	ent_type_count
};

enum ent_states {
	es_waiting      = 1 << 2,
	es_killed       = 1 << 3,
	es_modified     = 1 << 4,
	es_in_storage   = 1 << 5,
	es_spawned      = 1 << 6,
	es_pathfinding  = 1 << 7,
};

typedef uint32_t ent_id_t;

struct ent {
	struct point pos;
	ent_id_t id;
	enum ent_type type;
	uint32_t path;
	uint32_t target;
	uint16_t alignment;
	uint16_t loyalty;
	uint16_t age;
	uint8_t damage;
	uint8_t trav;
	uint8_t state;
};

void ent_init(struct ent *e);

typedef bool (*find_ent_predicate)(void *ctx, struct ent *e);
struct ent *find_ent(struct world *w, const struct point *p, void *ctx,
	find_ent_predicate epred);
#endif
