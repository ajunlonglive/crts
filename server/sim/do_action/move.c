#include "posix.h"

#include "server/sim/do_action.h"
#include "server/sim/action.h"
#include "server/sim/do_action/move.h"
#include "server/sim/ent.h"
#include "server/sim/worker.h"
#include "shared/sim/ent.h"
#include "shared/types/result.h"

enum result
do_action_move(struct simulation *sim, struct ent *e, struct sim_action *sa)
{
	if (e->state & es_waiting) {
		if (sa->act.workers_waiting >= sa->act.workers_assigned) {
			return rs_done;
		}
	} else if (e->state & es_pathfinding) {
		switch (ent_pathfind(&sim->world->chunks, e)) {
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
	} else {
		if (!ent_pgraph_set(&sim->world->chunks, e, &sa->act.range.pos)) {
			worker_unassign(sim, e, &sa->act);
		}
	}

	return rs_cont;
}
