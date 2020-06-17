#include "posix.h"

#ifndef CRTS_SERVER
#define CRTS_SERVER
#endif

#include <stdint.h>

#include "server/sim/action.h"
#include "server/sim/ent.h"
#include "server/sim/worker.h"
#include "shared/constants/globals.h"
#include "shared/sim/ent.h"

void
worker_assign(struct ent *e, struct action *act)
{
	act->workers_assigned++;

	e->target = e->subtask = 0;
	e->task = act->id;
	e->state &= ~(es_have_subtask | es_waiting);
	e->state |= es_have_task;
}

void
worker_unassign(struct simulation *sim, struct ent *e, struct action *act)
{
	e->state &= ~es_have_task;

	drop_held_ent(sim->world, e);
	pgraph_reset_all(e->pg);

	if (act != NULL) {
		act->workers_assigned--;
	}
}
