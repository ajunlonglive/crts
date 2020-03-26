#ifndef SERVER_SIM_DO_ACTION_H
#define SERVER_SIM_DO_ACTION_H

#include "server/sim/action.h"
#include "server/sim/sim.h"
#include "shared/sim/action.h"
#include "shared/sim/ent.h"

enum result do_action(struct simulation *sim, struct ent *e,
	struct sim_action *act);
struct ent * find_resource(struct world *w, enum ent_type t, struct point *p,
	struct circle *c);
enum result pickup_resources(struct simulation *sim, struct ent *e,
	enum ent_type resource, struct circle *c);
#endif
