#ifndef SERVER_SIM_WORKER_H
#define SERVER_SIM_WORKER_H

#include "server/sim/action.h"

void worker_unassign(struct simulation *sim, struct ent *e, struct action *work);
#endif
