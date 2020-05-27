#ifndef SERVER_SIM_DO_ACTION_H
#define SERVER_SIM_DO_ACTION_H

#include "server/sim/action.h"
#include "server/sim/sim.h"
#include "shared/sim/action.h"
#include "shared/sim/ent.h"
#include "shared/types/result.h"

enum result do_action(struct simulation *sim, struct ent *e,
	struct sim_action *act);
struct ent * find_resource(struct world *w, enum ent_type t, struct point *p,
	struct circle *c);
enum result pickup_resources(struct simulation *sim, struct ent *e,
	enum ent_type resource, struct circle *c);
void ent_pgraph_set(struct ent *e, const struct point *g);
void set_action_targets(struct sim_action *sa);
#endif
