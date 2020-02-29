#ifndef SERVER_SIM_DO_ACTION_H
#define SERVER_SIM_DO_ACTION_H

#include "server/sim/sim.h"
#include "shared/sim/action.h"

enum result do_action(struct simulation *sim, struct ent *e, struct sim_action *act);
#endif
