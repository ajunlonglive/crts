#ifndef SERVER_SIM_WORKER_H
#define SERVER_SIM_WORKER_H

#include "shared/sim/action.h"
#include "shared/sim/world.h"

struct ent *worker_find(const struct world *w, struct action *work);
void worker_assign(struct ent *e, struct action *work);
void worker_unassign(struct ent *e, struct action *work);
#endif
