#include "posix.h"

#include <stdint.h>

#include "server/sim/action.h"
#include "server/sim/ent.h"
#include "server/sim/worker.h"
#include "shared/constants/globals.h"
#include "shared/sim/ent.h"
#include "shared/pathfind/api.h"

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
	if (e->state & es_pathfinding) {
		hpa_finish(&sim->world->chunks, e->path);
	}

	e->state &= ~(es_have_task | es_pathfinding);

	drop_held_ent(sim->world, e);

	if (act != NULL) {
		act->workers_assigned--;
	}
}
