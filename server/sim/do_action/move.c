#ifndef CRTS_SERVER
#define CRTS_SERVER
#endif

#include "server/sim/action.h"
#include "server/sim/do_action/move.h"
#include "server/sim/pathfind/pathfind.h"
#include "server/sim/worker.h"
#include "shared/sim/ent.h"
#include "shared/types/result.h"

struct action_move_ctx {
	struct pgraph *pg;
};

_Static_assert(sizeof(struct action_move_ctx) <= SIM_ACTION_CTX_LEN,
	"struct action_move_ctx too big");

enum result
do_action_move(struct simulation *sim, struct ent *e, struct sim_action *sa)
{
	struct action_move_ctx *ctx = (void *)sa->ctx;

	if (!ctx->pg) {
		ctx->pg = pgraph_create(sim->world->chunks, &sa->act.range.center,
			e->type);
	}

	if (e->state & es_waiting) {
		if (sa->act.workers_waiting >= sa->act.workers_assigned) {
			pgraph_destroy(ctx->pg);

			return rs_done;
		} else {
			return rs_cont;
		}
	}

	switch (pathfind(ctx->pg, &e->pos)) {
	case rs_cont:
		e->state |= es_modified;
		break;
	case rs_fail:
		action_ent_blacklist(sa, e);
		worker_unassign(sim, e, &sa->act);
		break;
	case rs_done:
		e->state |= es_waiting;
		sa->act.workers_waiting++;
		break;
	}

	return rs_cont;
}
