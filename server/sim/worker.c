#ifndef CRTS_SERVER
#define CRTS_SERVER
#endif

#define _XOPEN_SOURCE 500

#include <stdint.h>

#include "server/sim/action.h"
#include "server/sim/worker.h"
#include "shared/sim/alignment.h"
#include "shared/sim/ent.h"

static bool
find_worker_pred(void *ctx, struct ent *e)
{
	const struct sim_action *sa = ctx;

	return e->idle
	       && e->alignment->max == sa->act.motivator
	       && !action_ent_blacklisted(sa, e);
}

struct ent *
worker_find(const struct world *w, struct sim_action *sa)
{
	return find_ent(w, &sa->act.range.center, sa, find_worker_pred);
}

void
worker_assign(struct ent *e, struct action *act)
{
	act->workers_assigned++;
	e->task = act->id;
	e->idle = 0;
}

void
worker_unassign(struct ent *e, struct action *act)
{
	e->task = 0;
	e->idle = true;
	e->wait = false;

	if (act != NULL) {
		act->workers_assigned--;
	}
}
