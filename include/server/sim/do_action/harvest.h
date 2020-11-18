#ifndef SERVER_SIM_DO_ACTION_HARVEST_H
#define SERVER_SIM_DO_ACTION_HARVEST_H

#include "server/sim/action.h"
#include "shared/types/result.h"

enum result do_action_harvest(struct simulation *sim, struct ent *e, struct sim_action *act);
void do_action_harvest_setup(struct simulation *sim, struct sim_action *act);
void do_action_harvest_teardown(struct sim_action *act);
#endif
