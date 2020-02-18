#ifndef SERVER_SIM_DO_ACTION_H
#define SERVER_SIM_DO_ACTION_H

#include "server/sim/sim.h"
#include "shared/sim/action.h"

enum action_result {
	ar_cont,
	ar_done,
	ar_fail,
};

int do_action(struct simulation *sim, struct ent *e, struct sim_action *act);
#endif
