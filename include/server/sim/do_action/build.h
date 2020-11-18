#ifndef SERVER_SIM_DO_ACTION_BUILD_H
#define SERVER_SIM_DO_ACTION_BUILD_H

#include "server/sim/action.h"
#include "shared/types/result.h"

void do_action_build_setup(struct simulation *sim, struct sim_action *sa);
void do_action_build_teardown(struct sim_action *sa);
enum result do_action_build(struct simulation *sim, struct ent *e, struct sim_action *sa);
#endif
