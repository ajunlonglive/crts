#ifndef SERVER_SIM_DO_ACTION_FIGHT_H
#define SERVER_SIM_DO_ACTION_FIGHT_H

#include "server/sim/action.h"
#include "shared/types/result.h"

enum result do_action_fight(struct simulation *sim, struct ent *e, struct sim_action *sa);
#endif
