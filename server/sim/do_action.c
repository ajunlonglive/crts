#ifndef CRTS_SERVER
#define CRTS_SERVER
#endif

#define _XOPEN_SOURCE 500

#include "server/sim/action.h"
#include "server/sim/do_action.h"
#include "server/sim/do_action/build.h"
#include "server/sim/do_action/carry.h"
#include "server/sim/do_action/fight.h"
#include "server/sim/do_action/harvest.h"
#include "server/sim/do_action/move.h"
#include "server/sim/terrain.h"
#include "shared/messaging/server_message.h"
#include "shared/types/result.h"
#include "shared/util/log.h"

struct find_resource_ctx {
	enum ent_type t;
	struct circle *range;
};

static bool
find_resource_pred(void *_ctx, struct ent *e)
{
	struct find_resource_ctx *ctx = _ctx;

	return ctx->t == e->type &&
	       (ctx->range ? point_in_circle(&e->pos, ctx->range) : true);
}

struct ent *
find_resource(struct world *w, enum ent_type t, struct point *p, struct circle *c)
{
	struct find_resource_ctx ctx = { t, c };

	return find_ent(w, p, &ctx, find_resource_pred);
}

enum result
pickup_resources(struct simulation *sim, struct ent *e, enum ent_type resource,
	struct circle *c)
{
	struct ent *res;
	enum result r;

	if (e->pg == NULL) {
		if ((res = find_resource(sim->world, resource, &e->pos, c)) != NULL) {
			e->pg = pgraph_create(sim->world->chunks, &res->pos);
			e->target = res->id;
		} else {
			L("failed to find resource");
			return rs_fail;
		}
	}

	switch (r = pathfind_and_update(sim, e->pg, e)) {
	case rs_done:
		if ((res = hdarr_get(sim->world->ents, &e->target)) != NULL
		    && !(res->state & es_killed)
		    && points_equal(&e->pos, &res->pos)) {
			L("ent %d picking up ent %x", e->id, e->state);
			e->holding = res->type;

			kill_ent(sim, res);
		}

	/* FALLTHROUGH */
	case rs_fail:
		pgraph_destroy(e->pg);
		e->target = 0;
		e->pg = NULL;
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
	default:
		return rs_done;
	}
}
