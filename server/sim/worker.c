#ifndef CRTS_SERVER
#define CRTS_SERVER
#endif

#define _XOPEN_SOURCE 500

#include <stdint.h>

#include "server/sim/worker.h"
#include "shared/sim/alignment.h"
#include "shared/sim/ent.h"

struct ent *
worker_find(const struct world *w, const struct action *work)
{
	size_t i;
	uint16_t dist, closest_dist = UINT16_MAX;
	struct ent *e, *worker = NULL;

	for (i = 0; i < w->ecnt; i++) {
		e = &w->ents[i];

		if (e->idle && e->alignment->max == work->motivator) {

			dist = distance_point_to_circle(&e->pos, &work->range);

			if (dist < closest_dist) {
				closest_dist = dist;
				worker = e;
			}
		}
	}

	return worker;
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

