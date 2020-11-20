#ifndef SERVER_SIM_DO_ACTION_CARRY_H
#define SERVER_SIM_DO_ACTION_CARRY_H

#include "server/sim/action.h"
#include "shared/types/result.h"

enum result do_action_carry(struct simulation *sim, struct ent *e, struct sim_action *sa);
void do_action_carry_setup(struct simulation *sim, struct sim_action *sa);
void do_action_carry_teardown(struct sim_action *sa);
#endif
