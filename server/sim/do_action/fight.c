#include "posix.h"

#ifndef CRTS_SERVER
#define CRTS_SERVER
#endif

#include "server/sim/action.h"
#include "server/sim/do_action.h"
#include "server/sim/do_action/fight.h"
#include "server/sim/ent.h"
#include "server/sim/pathfind/pathfind.h"
#include "server/sim/terrain.h"
#include "server/sim/worker.h"
#include "shared/constants/globals.h"
#include "shared/sim/ent.h"
#include "shared/types/result.h"
#include "shared/util/log.h"

struct find_enemey_pred_ctx {
	struct ent *e;
	struct circle *range;
};

static bool
find_enemy_pred(void *_ctx, struct ent *e)
{
	struct find_enemey_pred_ctx *ctx = _ctx;

	return point_in_circle(&e->pos, ctx->range) && gcfg.ents[e->type].animate
	       && e->alignment != ctx->e->alignment;
}

enum result
do_action_fight(struct simulation *sim, struct ent *e, struct sim_action *sa)
{
	struct ent *en;
	struct point p;
	struct find_enemey_pred_ctx ctx = { e, &sa->act.range };

	/* find target */

	if (!e->target) {
		if ((en = find_ent(sim->world, &e->pos, &ctx, find_enemy_pred))) {
			e->target = en->id;
		} else {
			return rs_fail;
		}
	} else if ((en = hdarr_get(sim->world->ents, &e->target))) {
		if (points_adjacent(&e->pos, &en->pos)) {
			damage_ent(sim, en, 1);

			return rs_cont;
		}
	} else {
		e->target = 0;
		return rs_cont;
	}

	/* pathfind to target if out of range */

	if (e->pg->unset) {
		if (find_adj_tile(sim->world->chunks, &en->pos, &p, NULL, -1,
			e->trav, NULL, tile_is_traversable)) {
			ent_pgraph_set(e, &p);
		} else {
			return rs_fail;
		}
	}

	switch (ent_pathfind(e)) {
	case rs_fail:
		worker_unassign(sim, e, &sa->act);
	/* FALLTHROUGH */
	case rs_done:
		e->pg->unset = true;
		break;
	case rs_cont:
		break;
	}

	return rs_cont;
}
