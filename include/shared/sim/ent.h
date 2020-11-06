#ifndef SHARED_SIM_ENT_H
#define SHARED_SIM_ENT_H

#include <stdbool.h>
#include <stdint.h>

#include "shared/math/geom.h"
#include "shared/sim/world.h"
#include "shared/types/result.h"

enum ent_type {
	et_none,
	et_worker,
	et_elf_corpse,
	et_deer,
	et_fish,
	et_vehicle_boat,
	et_resource_wood,
	et_resource_meat,
	et_resource_rock,
	et_resource_crop,
	et_storehouse,
	ent_type_count
};

#ifdef CRTS_SERVER
#include "shared/pathfind/api.h"

struct ent_lookup_ctx;
#endif

enum ent_states {
	es_have_subtask = 1 << 0,
	es_have_task    = 1 << 1,
	es_waiting      = 1 << 2,
	es_killed       = 1 << 3,
	es_modified     = 1 << 4,
	es_in_storage   = 1 << 5,
	es_hungry       = 1 << 6,
	es_spawned      = 1 << 7,
	es_pathfinding  = 1 << 8,
};

typedef uint32_t ent_id_t;

struct ent {
	struct point pos;

	ent_id_t id;
	enum ent_type type;
	uint16_t alignment;
	uint8_t damage;

#ifdef CRTS_SERVER
	uint8_t trav;

	struct ent_lookup_ctx *elctx;
	uint32_t path;
	enum ent_type holding;
	uint32_t target;
	uint16_t age;
	uint16_t subtask;
	uint16_t hunger;
	uint16_t state;
	uint8_t task;
#endif
};

void ent_init(struct ent *e);

typedef bool (*find_ent_predicate)(void *ctx, struct ent *e);
struct ent *find_ent(const struct world *w, const struct point *p, void *ctx,
	find_ent_predicate epred);
#endif
