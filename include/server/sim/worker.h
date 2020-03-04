#ifndef SERVER_SIM_WORKER_H
#define SERVER_SIM_WORKER_H

#include "server/sim/action.h"
#include "shared/sim/action.h"
#include "shared/sim/world.h"

struct ent *worker_find(const struct world *w, struct sim_action *sa);
void worker_assign(struct ent *e, struct action *work);
void worker_unassign(struct ent *e, struct action *work);
#endif
