#include "posix.h"

#include "server/sim/action.h"
#include "server/sim/do_action.h"
#include "server/sim/do_action/carry.h"
#include "server/sim/ent.h"
#include "server/sim/storehouse.h"
#include "server/sim/worker.h"
#include "shared/constants/globals.h"
#include "shared/sim/ent.h"
#include "shared/sim/tiles.h"
#include "shared/types/result.h"
#include "shared/util/log.h"

struct carry_target {
	struct point adj;
	ent_id_t ent;
};

struct action_carry_ctx {
	struct darr targets;
	struct chunks *cnks;
	bool searched;
};

_Static_assert(sizeof(struct action_carry_ctx) <= SIM_ACTION_CTX_LEN,
	"struct action_build_ctx too big");

static enum result
dropoff_resources(struct simulation *sim, struct ent *e, struct point *p)
{
	enum result r;

	if (!(e->state & es_pathfinding)) {
		struct storehouse_storage *st =
			nearest_storehouse(&sim->world->chunks, &e->pos,
				e->holding);
		struct point rp;
		if (st && find_adj_tile(&sim->world->chunks, &st->pos, &rp,
			NULL, -1, e->trav, NULL, tile_is_traversable)) {
			if (!ent_pgraph_set(&sim->world->chunks, e, &rp)) {
				return rs_fail;
			}
		} else {
			return rs_fail;
		}
	}

	switch (r = ent_pathfind(&sim->world->chunks, e)) {
	case rs_cont:
	case rs_fail:
		break;
	case rs_done:
	{
		struct point rp;

		if (!find_adj_tile(&sim->world->chunks, &e->pos, &rp, NULL,
			tile_storehouse, 0, NULL, NULL)) {
			return rs_fail;
		}

		struct storehouse_storage *st =
			get_storehouse_storage_at(&sim->world->chunks, &rp);

		if (!storehouse_store(st, e->holding)) {
			return rs_fail;
		}

		e->holding = et_none;
		break;
	}
	}

	return r;
}

static bool
carry_target_pred(struct ent *e, void *_ctx)
{
	struct find_resource_ctx *ctx = _ctx;

	bool matches = !gcfg.ents[e->type].animate && !gcfg.ents[e->type].phantom;

	if (matches && (!ctx->range || point_in_rect(&e->pos, ctx->range))) {

		struct action_carry_ctx *acctx = ctx->usr_ctx;
		struct point rp;

		if (find_adj_tile(acctx->cnks, &e->pos, &rp,
			NULL, -1, e->trav, NULL, tile_is_traversable)) {

			darr_push(&acctx->targets, &(struct carry_target){
				.ent = e->id, .adj = rp
			});

			return true;
		}
	}

	return false;
}

static bool
next_carry_target(struct simulation *sim, struct sim_action *sa, struct carry_target *tgt)
{
	struct action_carry_ctx *ctx = (void *)sa->ctx;

	if (ctx->targets.len) {
		goto pop_return;
	} else if (ctx->searched) {
		return false;
	}

	assert(!ctx->targets.len);

	L("setting %d carry targets", sa->act.workers_assigned);
	sa->elctx.init = true;
	sa->elctx.needed = 0;
	sa->elctx.origin = &sa->act.range.pos;
	sa->elctx.pred = carry_target_pred;
	sa->elctx.cb = NULL;

	struct find_resource_ctx frctx = {
		.t = 0, .range = &sa->act.range, .usr_ctx = ctx,
		.chunks = &sim->world->chunks
	};

	sa->elctx.usr_ctx = &frctx;

	ent_lookup(sim, &sa->elctx);
	ent_lookup_reset(&sa->elctx);

	L("found %ld", ctx->targets.len);

	ctx->searched = true;

pop_return:
	*tgt = *(struct carry_target *)darr_get(&ctx->targets, ctx->targets.len - 1);
	darr_del(&ctx->targets, ctx->targets.len - 1);

	return true;
}

void
do_action_carry_setup(struct simulation *sim, struct sim_action *sa)
{
	struct action_carry_ctx *ctx = (void *)sa->ctx;

	darr_init(&ctx->targets, sizeof(struct carry_target));

	ctx->cnks = &sim->world->chunks;
}

void
do_action_carry_teardown(struct sim_action *sa)
{
	struct action_carry_ctx *ctx = (void *)sa->ctx;

	darr_destroy(&ctx->targets);
}

enum result
do_action_carry(struct simulation *sim, struct ent *e, struct sim_action *sa)
{
	/* struct action_carry_ctx *ctx = (void *)sa->ctx; */

	if (e->holding) {
		switch (dropoff_resources(sim, e, &sa->act.range.pos)) {
		case rs_fail:
			worker_unassign(sim, e, &sa->act);
			break;
		default:
			break;
		}
	} else {
		if (!(e->state & es_pathfinding)) {
			struct carry_target tgt;

			if (next_carry_target(sim, sa, &tgt)
			    && ent_pgraph_set(&sim->world->chunks, e, &tgt.adj)) {
				e->target = tgt.ent;
			} else {
				worker_unassign(sim, e, &sa->act);
			}
		} else {
			switch (ent_pathfind(&sim->world->chunks, e)) {
			case rs_cont:
			case rs_fail:
				break;
			case rs_done:
				pickup_resource(sim, e, 0);
				break;
			}
		}

	}

	return rs_cont;
}
