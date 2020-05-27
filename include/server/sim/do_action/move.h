#ifndef SERVER_SIM_DO_ACTION_MOVE_H
#define SERVER_SIM_DO_ACTION_MOVE_H

#include "server/sim/action.h"
#include "shared/types/result.h"

enum result do_action_move(struct simulation *sim, struct ent *e, struct sim_action *sa);
#endif
