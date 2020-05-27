#ifndef SERVER_SIM_DO_ACTION_BUILD_H
#define SERVER_SIM_DO_ACTION_BUILD_H

#include "server/sim/action.h"
#include "shared/types/result.h"

enum result do_action_build(struct simulation *sim, struct ent *e, struct sim_action *sa);
#endif
