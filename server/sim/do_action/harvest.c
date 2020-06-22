#include "posix.h"

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


bool
tile_is_harvestable(enum tile t, uint8_t _)
{
	return gcfg.tiles[t].hardness > 0;
}

static enum result
goto_tile(struct simulation *sim, struct ent *e, struct sim_action *act)
{
	switch (pathfind(&act->pg, &e->pos)) {
	case rs_cont:
		e->state |= es_modified;
		break;
	case rs_fail:
		L("failed :(");
		/* action_ent_blacklist(act, e); // blacklist disabled */
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
	struct rectangle *r = &sa->act.range;
	struct action_harvest_ctx *ctx = (void *)sa->ctx;

	ctx->targets = 0;
	pgraph_reset_goals(&sa->pg);

	for (cp.x = r->pos.x; cp.x < r->pos.x + (int64_t)r->width; ++cp.x) {
		for (cp.y = r->pos.y; cp.y < r->pos.y + (int64_t)r->height; ++cp.y) {
			if (tile_is_harvestable(get_tile_at(sa->pg.chunks, &cp), 0)
			    && find_adj_tile(sa->pg.chunks, &cp, &rp, NULL, -1,
				    sa->pg.trav, NULL, tile_is_traversable)) {
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
	struct action_harvest_ctx *ctx = (void *)act->ctx;

	if (!ctx->targets) {
		set_harvest_targets(act);

		if (!ctx->targets) {
			return rs_done;
		}
	}

	if (!find_adj_tile(sim->world->chunks, &e->pos, &p, &act->act.range,
		0, -1, NULL, tile_is_harvestable)) {
		return goto_tile(sim, e, act);
	}

	ck = get_chunk_at(sim->world->chunks, &p);
	rp = point_sub(&p, &ck->pos);

	if (++ck->harvested[rp.x][rp.y] >= gcfg.tiles[ck->tiles[rp.x][rp.y]].hardness) {
		harvest_tile(sim->world, &p, 0, 0);
		set_harvest_targets(act);
	}

	return rs_cont;
}
