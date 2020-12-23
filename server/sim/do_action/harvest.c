#include "posix.h"

#include "server/sim/action.h"
#include "server/sim/do_action.h"
#include "server/sim/do_action/harvest.h"
#include "server/sim/ent.h"
#include "server/sim/worker.h"
#include "shared/constants/globals.h"
#include "shared/sim/tiles.h"
#include "shared/types/result.h"
#include "shared/util/log.h"
#include "tracy.h"

struct action_harvest_ctx {
	struct darr targets;
};

_Static_assert(sizeof(struct action_harvest_ctx) <= SIM_ACTION_CTX_LEN,
	"struct action_harvest_ctx too big");

bool
tile_is_harvestable(enum tile t, uint8_t _)
{
	return gcfg.tiles[t].hardness > 0;
}

static void
set_harvest_targets(struct simulation *sim, struct sim_action *sa)
{
	struct point cp, rp;
	struct rectangle *r = &sa->act.range;
	struct action_harvest_ctx *ctx = (struct action_harvest_ctx *)sa->ctx;

	assert(!ctx->targets.len);

	for (cp.x = r->pos.x; cp.x < r->pos.x + (int64_t)r->width; ++cp.x) {
		for (cp.y = r->pos.y; cp.y < r->pos.y + (int64_t)r->height; ++cp.y) {
			if (tile_is_harvestable(get_tile_at(&sim->world->chunks, &cp), 0)
			    && find_adj_tile(&sim->world->chunks, &cp, &rp, NULL, -1,
				    trav_land, NULL, tile_is_traversable)) {
				darr_push(&ctx->targets, &rp);
			}
		}
	}
}

void
do_action_harvest_setup(struct simulation *sim, struct sim_action *act)
{
	struct action_harvest_ctx *ctx = (struct action_harvest_ctx *)act->ctx;

	darr_init(&ctx->targets, sizeof(struct point));
}

void
do_action_harvest_teardown(struct sim_action *act)
{
	struct action_harvest_ctx *ctx = (struct action_harvest_ctx *)act->ctx;

	darr_destroy(&ctx->targets);
}

enum result
do_action_harvest(struct simulation *sim, struct ent *e, struct sim_action *act)
{
	TracyCZoneAutoS;
	struct chunk *ck;
	struct point p, rp;
	struct action_harvest_ctx *ctx = (struct action_harvest_ctx *)act->ctx;

	if (!ctx->targets.len) {
		set_harvest_targets(sim, act);

		if (!ctx->targets.len) {
			TracyCZoneAutoE;
			return rs_done;
		}
	}

	if (!(e->state & es_pathfinding)) {
		if (find_adj_tile(&sim->world->chunks, &e->pos, &p, &act->act.range,
			0, -1, NULL, tile_is_harvestable)) {
			goto harvest;
		}

		struct point p = *(struct point *)darr_get(&ctx->targets, ctx->targets.len - 1);
		darr_del(&ctx->targets, ctx->targets.len - 1);

		ent_pgraph_set(&sim->world->chunks, e, &p);
	} else {
		switch (ent_pathfind(&sim->world->chunks, e)) {
		case rs_fail:
			worker_unassign(sim, e, &act->act);
			break;
		case rs_done:
		case rs_cont:
			break;
		}
	}

	TracyCZoneAutoE;
	return rs_cont;
harvest:
	ck = get_chunk_at(&sim->world->chunks, &p);
	rp = point_sub(&p, &ck->pos);

	if (++ck->harvested[rp.x][rp.y] >= gcfg.tiles[ck->tiles[rp.x][rp.y]].hardness) {
		harvest_tile(sim->world, &p, 0, 0);
		/* set_harvest_targets(sim, act); */
	}

	TracyCZoneAutoE;
	return rs_cont;
}
