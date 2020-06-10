#ifndef CRTS_SERVER
#define CRTS_SERVER
#endif

#include "server/sim/action.h"
#include "server/sim/do_action.h"
#include "server/sim/do_action/build.h"
#include "server/sim/do_action/carry.h"
#include "server/sim/do_action/dismount.h"
#include "server/sim/do_action/fight.h"
#include "server/sim/do_action/harvest.h"
#include "server/sim/do_action/mount.h"
#include "server/sim/do_action/move.h"
#include "server/sim/ent.h"
#include "server/sim/ent_lookup.h"
#include "server/sim/pathfind/pathfind.h"
#include "server/sim/terrain.h"
#include "shared/constants/globals.h"
#include "shared/messaging/server_message.h"
#include "shared/types/result.h"
#include "shared/util/log.h"

struct find_resource_ctx {
	enum ent_type t;
	struct circle *range;
	struct ent *e;
};

static bool
find_resource_pred(struct ent *e, void *_ctx)
{
	struct find_resource_ctx *ctx = _ctx;

	return ctx->t == e->type &&
	       (ctx->range ? point_in_circle(&e->pos, ctx->range) : true);
}

static void
find_resource_cb(struct ent *e, void *_ctx)
{
	struct find_resource_ctx *ctx = _ctx;
	ctx->e = e;
}

static enum result
find_resource(struct simulation *sim, struct ent *e,
	enum ent_type t, struct circle *c, struct ent **res)
{
	struct find_resource_ctx ctx = { t, c, NULL };

	if (!e->elctx->init) {
		pgraph_reset_goals(e->pg);

		e->pg->trav = e->trav;
		pgraph_add_goal(e->pg, &e->pos);

		e->elctx->init = true;
		e->elctx->needed = 1;
		e->elctx->origin = &e->pos;
		e->elctx->pred = find_resource_pred;
		e->elctx->cb = find_resource_cb;
		e->elctx->needed = 1;
	}

	e->elctx->usr_ctx = &ctx;

	enum result r = ent_lookup(sim, e->elctx);

	if (e->elctx->found) {
		*res = ctx.e;
		return rs_done;
	} else if (r == rs_cont) {
		return rs_cont;
	} else {
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
	enum ent_type resource, struct circle *c)
{
	struct ent *res;
	enum result r;

	if (e->pg->unset) {
		switch (find_resource(sim, e, resource, c, &res)) {
		case rs_cont:
			L("looking for resource");
			return rs_cont;
		case rs_fail:
			L("failed to find resource");
			return rs_fail;
		case rs_done:
			L("found resource!");
			pgraph_reset_all(e->pg);
			ent_lookup_reset(e->elctx);

			e->target = res->id;
			ent_pgraph_set(e, &res->pos);
		}
	}

	switch (r = ent_pathfind(e)) {
	case rs_done:
		if ((res = hdarr_get(sim->world->ents, &e->target)) != NULL
		    && !(res->state & es_killed)
		    && points_equal(&e->pos, &res->pos)) {
			e->holding = res->type;

			kill_ent(sim, res);
		}

	/* FALLTHROUGH */
	case rs_fail:
		e->target = 0;
		e->pg->unset = true;
		ent_lookup_reset(e->elctx);
		break;
	case rs_cont:
		break;
	}

	return r;
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
	case at_mount:
		return do_action_mount(sim, e, act);
	case at_dismount:
		return do_action_dismount(sim, e, act);
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
		pgraph_add_goal(&sa->pg, &sa->act.range.center);
		break;
	}
}
