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

static struct ent *
find_resource(struct simulation *sim, struct ent *e, enum ent_type t, struct circle *c)
{
	struct find_resource_ctx ctx = { t, c, NULL };

	pgraph_reset_goals(e->pg);

	e->pg->trav = e->trav;
	pgraph_add_goal(e->pg, &e->pos);

	if (ent_lookup(sim, e->pg, &ctx, find_resource_pred, find_resource_cb,
		1, &e->pos)) {
		return ctx.e;
	} else {
		return NULL;
	}

	pgraph_reset_all(e->pg);
}

void
ent_pgraph_set(struct ent *e, const struct point *g)
{
	e->pg->trav = e->trav;
	pgraph_reset_goals(e->pg);
	pgraph_add_goal(e->pg, g);
}

enum result
pickup_resources(struct simulation *sim, struct ent *e, enum ent_type resource,
	struct circle *c)
{
	struct ent *res;
	enum result r;

	if (e->pg->unset) {
		if ((res = find_resource(sim, e, resource, c)) != NULL) {
			ent_pgraph_set(e, &res->pos);
			e->target = res->id;
		} else {
			return rs_fail;
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
