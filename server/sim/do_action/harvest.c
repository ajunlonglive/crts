#ifndef CRTS_SERVER
#define CRTS_SERVER
#endif

#include "server/sim/action.h"
#include "server/sim/do_action.h"
#include "server/sim/do_action/harvest.h"
#include "server/sim/ent.h"
#include "server/sim/pathfind/pathfind.h"
#include "server/sim/terrain.h"
#include "server/sim/worker.h"
#include "shared/constants/globals.h"
#include "shared/types/result.h"
#include "shared/util/log.h"

struct action_harvest_ctx {
	uint32_t targets;
};

_Static_assert(sizeof(struct action_harvest_ctx) <= SIM_ACTION_CTX_LEN,
	"struct action_harvest_ctx too big");

static enum result
goto_tile(struct simulation *sim, struct ent *e, struct sim_action *act, enum tile tgt)
{
	switch (pathfind(&act->pg, &e->pos)) {
	case rs_cont:
		e->state |= es_modified;
		break;
	case rs_fail:
		L("failed :(");
		action_ent_blacklist(act, e);
		worker_unassign(sim, e, &act->act);
	/* FALLTHROUGH */
	case rs_done:
		break;
	}

	return rs_cont;
}

void
set_harvest_targets(struct sim_action *sa)
{
	struct point cp, rp;
	struct circle *c = &sa->act.range;
	struct action_harvest_ctx *ctx = (void *)sa->ctx;

	ctx->targets = 0;
	pgraph_reset_goals(&sa->pg);

	for (cp.x = c->center.x - c->r; cp.x < c->center.x + c->r; ++cp.x) {
		for (cp.y = c->center.y - c->r; cp.y < c->center.y + c->r; ++cp.y) {
			if (point_in_circle(&cp, c)
			    && get_tile_at(sa->pg.chunks, &cp) == sa->act.tgt
			    && find_adj_tile(sa->pg.chunks, &cp, &rp, NULL, -1,
				    sa->pg.trav, tile_is_traversable)) {
				L("adding goal: %d, %d", rp.x, rp.y);
				++ctx->targets;
				pgraph_add_goal(&sa->pg, &rp);
			}
		}
	}
}

enum result
do_action_harvest(struct simulation *sim, struct ent *e, struct sim_action *act)
{
	struct chunk *ck;
	struct point p, rp;
	enum tile tgt_tile = act->act.tgt;
	struct action_harvest_ctx *ctx = (void *)act->ctx;

	if (!ctx->targets) {
		set_harvest_targets(act);

		if (!ctx->targets) {
			return rs_done;
		}
	}

	if (!find_adj_tile(sim->world->chunks, &e->pos, &p, &act->act.range,
		tgt_tile, -1, NULL)) {
		return goto_tile(sim, e, act, tgt_tile);
	}

	ck = get_chunk_at(sim->world->chunks, &p);
	rp = point_sub(&p, &ck->pos);

	if (++ck->harvested[rp.x][rp.y] >= gcfg.tiles[act->act.tgt].hardness) {
		destroy_tile(sim->world, &p);
		set_harvest_targets(act);
	}

	return rs_cont;
}
