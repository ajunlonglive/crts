#ifndef CRTS_SERVER
#define CRTS_SERVER
#endif

#include "server/sim/action.h"
#include "server/sim/do_action/move.h"
#include "server/sim/ent.h"
#include "server/sim/pathfind/pathfind.h"
#include "server/sim/worker.h"
#include "shared/sim/ent.h"
#include "shared/types/result.h"

enum result
do_action_move(struct simulation *sim, struct ent *e, struct sim_action *sa)
{
	if (e->state & es_waiting) {
		if (sa->act.workers_waiting >= sa->act.workers_assigned) {
			return rs_done;
		} else {
			return rs_cont;
		}
	}

	switch (pathfind(&sa->pg, &e->pos)) {
	case rs_cont:
		e->state |= es_modified;
		break;
	case rs_fail:
		worker_unassign(sim, e, &sa->act);
		break;
	case rs_done:
		e->state |= es_waiting;
		sa->act.workers_waiting++;
		break;
	}

	return rs_cont;
}
