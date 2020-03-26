#ifndef CRTS_SERVER
#define CRTS_SERVER
#endif

#include "server/sim/action.h"
#include "server/sim/do_action.h"
#include "server/sim/do_action/carry.h"
#include "shared/sim/ent.h"
#include "shared/types/result.h"
#include "shared/util/log.h"

static enum result
dropoff_resources(struct simulation *sim, struct ent *e, struct point *p)
{
	enum result r;

	if (e->pg == NULL) {
		e->pg = pgraph_create(sim->world->chunks, p);
	}

	switch (r = pathfind_and_update(sim, e->pg, e)) {
	case rs_cont:
		break;
	case rs_fail:
	/* set_tile_inacessable(&act->hash, &act->local->goal); */
	/* FALLTHROUGH */
	case rs_done:
		pgraph_destroy(e->pg);
		e->pg = NULL;
		break;
	}

	return r;
}

enum result
do_action_carry(struct simulation *sim, struct ent *e, struct sim_action *sa)
{
	if (sa->act.workers_waiting >= sa->act.workers_assigned) {
		return rs_done; /* all ents  drop their item when unassigned */
	} else if (e->state & es_waiting) {
		return rs_cont;
	}

	if (e->holding) {
		switch (dropoff_resources(sim, e, &sa->act.source.center)) {
		case rs_cont:
			break;
		case rs_done:
			e->state |= es_waiting;
			++sa->act.workers_waiting;
			break;
		case rs_fail:
			return rs_fail;
		}
	} else {
		switch (pickup_resources(sim, e, sa->act.tgt, &sa->act.range)) {
		case rs_cont:
		case rs_done:
			break;
		case rs_fail:
			e->state |= es_waiting;
			++sa->act.workers_waiting;
			break;
		}
	}

	return rs_cont;
}
