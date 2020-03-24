#ifndef CRTS_SERVER
#define CRTS_SERVER
#endif

#define _XOPEN_SOURCE 500

#include <stdint.h>

#include "server/sim/action.h"
#include "server/sim/worker.h"
#include "shared/constants/globals.h"
#include "shared/sim/ent.h"

static bool
find_worker_pred(void *ctx, struct ent *e)
{
	const struct sim_action *sa = ctx;

	return gcfg.ents[e->type].animate
	       && !(e->state & (es_killed | es_have_task))
	       && e->alignment == sa->act.motivator
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

	e->target = e->subtask = 0;
	e->task = act->id;
	e->state &= ~(es_have_subtask | es_waiting);
	e->state |= es_have_task;
}

void
worker_unassign(struct simulation *sim, struct ent *e, struct action *act)
{
	struct ent *drop;
	e->state &= ~es_have_task;

	if (e->holding) {
		drop = spawn_ent(sim);
		drop->pos = e->pos;
		drop->type = e->holding;

		e->holding = 0;
	}

	if (act != NULL) {
		act->workers_assigned--;
	}
}
