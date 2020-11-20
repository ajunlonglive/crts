#include "posix.h"

#ifndef CRTS_SERVER
#define CRTS_SERVER
#endif

#include "server/sim/action.h"
#include "server/sim/do_action.h"
#include "server/sim/do_action/build.h"
#include "server/sim/do_action/carry.h"
#include "server/sim/do_action/fight.h"
#include "server/sim/do_action/harvest.h"
#include "server/sim/do_action/move.h"
#include "server/sim/ent.h"
#include "server/sim/ent_lookup.h"
#include "server/sim/storehouse.h"
#include "shared/constants/globals.h"
#include "shared/sim/tiles.h"
#include "shared/types/result.h"
#include "shared/util/log.h"
#include "shared/util/util.h"
#include "tracy.h"

static bool
find_resource_pred(struct ent *e, void *_ctx)
{
	struct find_resource_ctx *ctx = _ctx;

	bool matches = false;

	if (ctx->t == e->type) {
		matches = true;
	} else if (!ctx->t
		   && !gcfg.ents[e->type].animate
		   && !gcfg.ents[e->type].phantom) {
		matches = true;
	} else if (e->type == et_storehouse
		   && storehouse_contains(
			   get_storehouse_storage_at(ctx->chunks, &e->pos),
			   ctx->t)) {
		matches = true;
	}

	return matches && (!ctx->range || point_in_rect(&e->pos, ctx->range));
}

static void
find_resource_cb(struct ent *e, void *_ctx)
{
	struct find_resource_ctx *ctx = _ctx;
	ctx->usr_ctx = e;
}

static bool
find_resource(struct simulation *sim, struct ent_lookup_ctx *elctx,
	const struct point *origin, enum ent_type t, struct rectangle *r,
	struct ent **res)
{
	TracyCZoneAutoS;
	struct find_resource_ctx ctx = {
		.t = t, .range = r, .usr_ctx = NULL,
		.chunks = &sim->world->chunks
	};

	if (!elctx->init) {
		elctx->init = true;
		elctx->needed = 1;
		elctx->origin = origin;
		elctx->pred = find_resource_pred;
		elctx->cb = find_resource_cb;
	}

	elctx->usr_ctx = &ctx;

	ent_lookup(sim, elctx);

	if (elctx->found) {
		*res = ctx.usr_ctx;
		TracyCZoneAutoE;
		return true;
	} else {
		TracyCZoneAutoE;
		return false;
	}
}

bool
pickup_resource(struct simulation *sim, struct ent *e, enum ent_type resource)
{
	struct ent *res;

	ent_id_t tgt = e->target;
	e->target = 0;

	if ((res = hdarr_get(&sim->world->ents, &tgt))
	    && !(res->state & es_killed)
	    && points_adjacent(&e->pos, &res->pos)) {
		if (res->type == et_storehouse) {
			struct storehouse_storage *st =
				get_storehouse_storage_at(
					&sim->world->chunks, &res->pos);

			if (storehouse_take(st, resource)) {
				e->holding = resource;
			} else {
				return false;
			}
		} else {
			e->holding = res->type;

			kill_ent(sim, res);
		}

		return true;
	} else {
		return false;
	}
}

enum result
pickup_resources(struct simulation *sim, struct ent_lookup_ctx *elctx,
	struct ent *e, enum ent_type resource, struct rectangle *r)
{
	struct ent *res;
	enum result result;

	if (!(e->state & es_pathfinding)) {
		ent_lookup_reset(elctx);

		if (find_resource(sim, elctx, &e->pos, resource, r, &res)) {
			struct point rp;

			if (elctx->exclude) {
				hash_set(elctx->exclude, &res->id, 1);
			}

			if (find_adj_tile(&sim->world->chunks, &res->pos, &rp,
				NULL, -1, e->trav, NULL, tile_is_traversable)) {

				ent_lookup_reset(elctx);

				e->target = res->id;

				if (!ent_pgraph_set(&sim->world->chunks, e, &rp)) {
					return rs_fail;
				}
			} else {
				ent_lookup_reset(elctx);
				return rs_fail;
			}
		} else {
			return rs_fail;
		}
	}

	assert(e->state & es_pathfinding);

	switch (result = ent_pathfind(&sim->world->chunks, e)) {
	case rs_done:
		if (!pickup_resource(sim, e, resource)) {
			result = rs_cont;
		}
		ent_lookup_reset(elctx);
		break;
	case rs_fail:
		e->target = 0;
		ent_lookup_reset(elctx);
		break;
	case rs_cont:
		break;
	}

	return result;
}

void
do_action_setup(struct simulation *sim, struct sim_action *act)
{
	switch (act->act.type) {
	case at_harvest:
		do_action_harvest_setup(sim, act);
		act->do_action = do_action_harvest;
		act->do_action_teardown = do_action_harvest_teardown;
		break;
	case at_build:
		do_action_build_setup(sim, act);
		act->do_action = do_action_build;
		act->do_action_teardown = do_action_build_teardown;
		break;
	case at_move:
		/* do_action_move_setup(sim, act); */
		act->do_action = do_action_move;
		/* act->do_action_teardown = do_action_move_teardown; */
		break;
	case at_fight:
		/* do_action_fight_setup(sim, act); */
		act->do_action = do_action_fight;
		/* act->do_action_teardown = do_action_fight_teardown; */
		break;
	case at_carry:
		do_action_carry_setup(sim, act);
		act->do_action = do_action_carry;
		act->do_action_teardown = do_action_carry_teardown;
		break;
	default:
		assert(false);
	}
}

uint32_t
estimate_work(struct sim_action *sa, uint32_t avail)
{
	switch (sa->act.type) {
	case at_harvest:
		return clamp(rect_area(&sa->act.range) / 4, 1, avail);
	case at_build:
		return clamp(rect_area(&sa->act.range) / 8, 1, avail);
	case at_move:
		return clamp(avail / 4, 1, avail);
	default:
		return clamp(rect_area(&sa->act.range) / 4, 1, avail);
	}
}
