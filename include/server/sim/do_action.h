#ifndef SERVER_SIM_DO_ACTION_H
#define SERVER_SIM_DO_ACTION_H

#include "server/sim/action.h"
#include "server/sim/sim.h"
#include "shared/sim/action.h"
#include "shared/sim/ent.h"
#include "shared/types/result.h"

struct find_resource_ctx {
	struct rectangle *range;
	void *usr_ctx;
	struct chunks *chunks;
	enum ent_type t;
};

void do_action_setup(struct simulation *sim, struct sim_action *act);
enum result pickup_resources(struct simulation *sim, struct ent_lookup_ctx *elctx,
	struct ent *e, enum ent_type resource, struct rectangle *r);
uint32_t estimate_work(struct sim_action *sa, uint32_t avail);
bool pickup_resource(struct simulation *sim, struct ent *e, enum ent_type resource);
#endif
