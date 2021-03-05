#include "posix.h"

#include <stdint.h>

#include "server/sim/action.h"
#include "server/sim/ent.h"
#include "server/sim/worker.h"
#include "shared/constants/globals.h"
#include "shared/sim/ent.h"
#include "shared/pathfind/api.h"

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
