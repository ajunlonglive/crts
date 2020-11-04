#ifndef SERVER_SIM_DO_ACTION_H
#define SERVER_SIM_DO_ACTION_H

#include "server/sim/action.h"
#include "server/sim/sim.h"
#include "shared/sim/action.h"
#include "shared/sim/ent.h"
#include "shared/types/result.h"

enum result do_action(struct simulation *sim, struct ent *e,
	struct sim_action *act);
enum result pickup_resources(struct simulation *sim, struct ent *e,
	enum ent_type resource, struct rectangle *r);
void ent_pgraph_set(struct chunks *cnks, struct ent *e, const struct point *g);
void set_action_targets(struct sim_action *sa);
uint32_t estimate_work(struct sim_action *sa, uint32_t avail);
#endif
