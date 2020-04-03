#ifndef SERVER_SIM_DO_ACTION_HARVEST_H
#define SERVER_SIM_DO_ACTION_HARVEST_H

#include "server/sim/action.h"
#include "server/sim/sim.h"

void set_harvest_targets(struct sim_action *sa);
enum result do_action_harvest(struct simulation *sim, struct ent *e, struct sim_action *act);
#endif
