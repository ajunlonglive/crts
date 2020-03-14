#ifndef __SIM_ENT_H
#define __SIM_ENT_H

#include <stdbool.h>
#include <stdint.h>

#include "shared/math/geom.h"
#include "shared/sim/world.h"

#ifdef CRTS_SERVER
#include "server/sim/pathfind/pgraph.h"
#endif

enum ent_type {
	et_none,
	et_worker,
	et_resource_wood,
	et_resource_rock,
	ent_type_count
};

struct ent {
	struct point pos;

	uint32_t id;
	enum ent_type type;
	uint8_t damage;

#ifdef CRTS_SERVER
	struct alignment *alignment;
	struct pgraph *pg;
	enum ent_type holding;
	uint32_t target;
	uint8_t satisfaction;
	uint8_t task;
	bool idle;
	bool wait;
	bool dead;
	bool changed;
#else
	uint8_t alignment;
#endif
};

void ent_init(struct ent *e);

typedef bool (*find_ent_predicate)(void *ctx, struct ent *e);
struct ent *find_ent(const struct world *w, const struct point *p, void *ctx, find_ent_predicate epred);
#endif
