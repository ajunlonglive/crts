#ifndef SHARED_SIM_ENT_H
#define SHARED_SIM_ENT_H

#include <stdbool.h>
#include <stdint.h>

#include "shared/math/geom.h"
#include "shared/sim/world.h"

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
	ent_type_count
};

#ifdef CRTS_SERVER
#include "server/sim/pathfind/pgraph.h"
#endif

enum ent_states {
	es_have_subtask = 1 << 0,
	es_have_task    = 1 << 1,
	es_waiting      = 1 << 2,
	es_killed       = 1 << 3,
	es_modified     = 1 << 4,
};

typedef uint32_t ent_id_t;

struct ent {
	struct point pos;

	ent_id_t id;
	enum ent_type type;
	uint8_t damage;
	uint8_t alignment;

#ifdef CRTS_SERVER
	uint8_t trav;

	struct pgraph *pg;
	enum ent_type holding;
	enum ent_type riding;
	uint32_t target;
	uint16_t age;
	uint8_t satisfaction;
	uint8_t task;
	uint8_t subtask;
	uint8_t state;
#endif
};

void ent_init(struct ent *e);

typedef bool (*find_ent_predicate)(void *ctx, struct ent *e);
struct ent *find_ent(const struct world *w, const struct point *p, void *ctx,
	find_ent_predicate epred);
#endif
