#ifndef CRTS_SERVER
#define CRTS_SERVER
#endif

#define _XOPEN_SOURCE 500

#include <stdint.h>

#include "server/sim/worker.h"
#include "shared/sim/alignment.h"
#include "shared/sim/ent.h"

static bool
find_worker_pred(void *ctx, struct ent *e)
{
	const struct action *work = ctx;

	return e->idle && e->alignment->max == work->motivator;
}

struct ent *
worker_find(const struct world *w, struct action *work)
{
	return find_ent(w, &work->range.center, work, find_worker_pred);
}

void
worker_assign(struct ent *e, struct action *act)
{
	act->workers_assigned++;
	if (point_in_circle(&e->pos, &act->range)) {
		act->workers_in_range++;
	}
	e->task = act->id;
	e->idle = 0;
}

void
worker_unassign(struct ent *e, struct action *act)
{
	e->task = -1;
	e->idle = 1;

	if (act != NULL) {
		act->workers_assigned--;
		act->workers_in_range--;
	}
}
