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
#include "shared/pathfind/pathfind.h"
#include "shared/sim/tiles.h"
#include "shared/types/result.h"
#include "shared/util/log.h"
#include "shared/util/util.h"
#include "tracy.h"

struct find_resource_ctx {
	struct rectangle *range;
	struct ent *e;
	struct chunks *chunks;
	enum ent_type t;
};

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
	ctx->e = e;
}

static enum result
find_resource(struct simulation *sim, struct ent *e,
	enum ent_type t, struct rectangle *r, struct ent **res)
{
	TracyCZoneAutoS;
	struct find_resource_ctx ctx = {
		.t = t, .range = r, .e = NULL,
		.chunks = &sim->world->chunks
	};

	if (!e->elctx->init) {
		pgraph_reset_all(e->pg);

		e->pg->trav = e->trav;
		pgraph_add_goal(e->pg, &e->pos);

		e->elctx->init = true;
		e->elctx->needed = 1;
		e->elctx->origin = &e->pos;
		e->elctx->pred = find_resource_pred;
		e->elctx->cb = find_resource_cb;
	}

	e->elctx->usr_ctx = &ctx;

	enum result result = ent_lookup(sim, e->elctx);

	if (e->elctx->found) {
		*res = ctx.e;
		TracyCZoneAutoE;
		return rs_done;
	} else if (result == rs_cont) {
		TracyCZoneAutoE;
		return rs_cont;
	} else {
		TracyCZoneAutoE;
		return rs_fail;
	}
}

void
ent_pgraph_set(struct ent *e, const struct point *g)
{
	e->pg->trav = e->trav;
	pgraph_reset_goals(e->pg);
	pgraph_add_goal(e->pg, g);
}

enum result
pickup_resources(struct simulation *sim, struct ent *e,
	enum ent_type resource, struct rectangle *r)
{
	struct ent *res;
	enum result result;

	if (e->pg->unset) {
		ent_lookup_reset(e->elctx);

		switch (find_resource(sim, e, resource, r, &res)) {
		case rs_cont:
			return rs_cont;
		case rs_fail:
			e->pg->unset = true;
			return rs_fail;
		case rs_done:
		{
			struct point rp;

			if (find_adj_tile(&sim->world->chunks, &res->pos, &rp,
				NULL, -1, e->trav, NULL, tile_is_traversable)) {

				pgraph_reset_all(e->pg);
				ent_lookup_reset(e->elctx);

				e->target = res->id;
				ent_pgraph_set(e, &rp);
			} else {
				e->pg->unset = true;
				ent_lookup_reset(e->elctx);
				return rs_fail;
			}
		}
		}
	}

	switch (result = ent_pathfind(e)) {
	case rs_done:
		if ((res = hdarr_get(sim->world->ents, &e->target)) != NULL
		    && !(res->state & es_killed)
		    && points_adjacent(&e->pos, &res->pos)) {
			if (res->type == et_storehouse) {
				struct storehouse_storage *st =
					get_storehouse_storage_at(
						&sim->world->chunks, &res->pos);

				if (storehouse_take(st, resource)) {
					e->holding = resource;
				} else {
					result = rs_cont;
				}
			} else {
				e->holding = res->type;

				kill_ent(sim, res);
			}
		}
		e->target = 0;
		e->pg->unset = true;
		ent_lookup_reset(e->elctx);
		break;
	case rs_fail:
		e->target = 0;
		e->pg->unset = true;
		ent_lookup_reset(e->elctx);
		break;
	case rs_cont:
		break;
	}

	return result;
}

enum result
do_action(struct simulation *sim, struct ent *e, struct sim_action *act)
{
	switch (act->act.type) {
	case at_harvest:
		return do_action_harvest(sim, e, act);
	case at_build:
		return do_action_build(sim, e, act);
	case at_move:
		return do_action_move(sim, e, act);
	case at_fight:
		return do_action_fight(sim, e, act);
	case at_carry:
		return do_action_carry(sim, e, act);
	default:
		return rs_done;
	}
}

void
set_action_targets(struct sim_action *sa)
{
	switch (sa->act.type) {
	case at_harvest:
		set_harvest_targets(sa);
		break;
	default:
		sa->pg.trav = trav_land;
		pgraph_add_goal(&sa->pg, &sa->act.range.pos);
		break;
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
