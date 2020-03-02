#ifndef SERVER_SIM_DO_ACTION_H
#define SERVER_SIM_DO_ACTION_H

#include "server/sim/sim.h"
#include "shared/sim/action.h"
#include "shared/sim/ent.h"

enum result do_action(struct simulation *sim, struct ent *e, struct sim_action *act);
struct ent *find_resource(struct world *w, enum ent_type t, struct point *p);
void update_tile(struct simulation *sim, const struct point *p, enum tile t);
#endif
