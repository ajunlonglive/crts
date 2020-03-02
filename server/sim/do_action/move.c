#ifndef CRTS_SERVER
#define CRTS_SERVER
#endif

#include "server/sim/action.h"
#include "server/sim/do_action/move.h"
#include "shared/sim/ent.h"
#include "shared/types/result.h"

enum result
do_action_move(struct simulation *sim, struct ent *e, struct sim_action *sa)
{
	if (e->wait) {
		if (sa->act.workers_waiting >= sa->act.workers_assigned) {
			return rs_done;
		} else {
			return rs_cont;
		}
	}

	switch (pathfind_and_update(sim, sa->global, e)) {
	case rs_cont:
		break;
	case rs_fail:
		return rs_fail;
		break;
	case rs_done:
		e->wait = true;
		sa->act.workers_waiting++;
		break;
	}

	return rs_cont;
}
