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
	ent_type_count
};

struct ent {
	enum ent_type type;
	uint32_t id;
	struct point pos;

#ifdef CRTS_SERVER
	struct alignment *alignment;
	struct pgraph *pg;
	bool idle;
	enum ent_type holding;
	uint8_t satisfaction;
	uint8_t task;
	bool wait;
#else
	uint8_t alignment;
#endif
};

void ent_init(struct ent *e);

typedef bool (*find_ent_predicate)(void *ctx, struct ent *e);
struct ent *find_ent(const struct world *w, const struct point *p, void *ctx, find_ent_predicate epred);
#endif
