#ifndef CRTS_SERVER
#define CRTS_SERVER
#endif

#include "server/sim/action.h"
#include "server/sim/do_action.h"
#include "server/sim/do_action/build.h"
#include "server/sim/terrain.h"
#include "shared/constants/globals.h"
#include "shared/messaging/server_message.h"
#include "shared/types/result.h"
#include "shared/util/log.h"

struct reposition_ents_ctx {
	struct simulation *sim;
	const struct blueprint *blp;
	struct rectangle lot;
};

static enum iteration_result
reposition_ents(void *_ctx, void *_e)
{
	struct ent *e = _e;
	struct reposition_ents_ctx *ctx = _ctx;
	bool repos, didrepos = false;

	if (!point_in_rect(&e->pos, &ctx->lot)) {
		return ir_cont;
	}

	do {
		repos = false;

		if (!is_traversable(ctx->sim->world->chunks, &e->pos)) {
			didrepos = repos = true;
			e->pos.x++;
		}
	} while (repos);

	if (didrepos) {
		queue_push(ctx->sim->outbound, sm_create(server_message_ent, e));
	}

	return ir_cont;
}

static void
build_building(struct simulation *sim, const struct point *p, enum building b)
{
	size_t i;
	const struct blueprint *blp = &gcfg.blueprints[b];
	struct point rp;
	struct reposition_ents_ctx ctx = { sim, blp, blp->lot };

	ctx.lot.pos = point_add(&blp->lot.pos, p);

	for (i = 0; i < blp->len; ++i) {
		rp = point_add(p, &blp->blocks[i].p);
		update_tile(sim->world->chunks, &rp, blp->blocks[i].t);
	}

	hdarr_for_each(sim->world->ents, &ctx, reposition_ents);
}

static enum result
deliver_resources(struct simulation *sim, struct ent *e, struct sim_action *sa)
{
	if (sa->local == NULL) {
		sa->local = pgraph_create(sim->world->chunks, &sa->act.range.center);
	}

	switch (pathfind_and_update(sim, sa->local, e)) {
	case rs_done:
		e->holding = et_none;
		sa->resources++;
		L("sa->resource: %d", sa->resources);
		break;
	case rs_cont:
		break;
	case rs_fail:
		return rs_fail;
	}

	return rs_cont;
}

static enum result
pickup_resources(struct simulation *sim, struct ent *e, struct sim_action *sa)
{
	struct ent *wood;

	if (e->pg == NULL) {
		if ((wood = find_resource(sim->world, et_resource_wood, &e->pos)) != NULL) {
			e->pg = pgraph_create(sim->world->chunks, &wood->pos);
		} else {
			return rs_fail;
		}
	}

	switch (pathfind_and_update(sim, e->pg, e)) {
	case rs_done:
		if ((wood = find_resource(sim->world, et_resource_wood, &e->pos)) != NULL
		    && points_equal(&e->pos, &wood->pos)) {
			e->holding = et_resource_wood;

			sim_destroy_ent(sim, wood);
		} else {
			pgraph_destroy(e->pg);
			e->pg = NULL;
		}

		break;
	case rs_cont:
		break;
	case rs_fail:
		return rs_fail;
	}

	return rs_cont;
}

enum result
do_action_build(struct simulation *sim, struct ent *e, struct sim_action *sa)
{
	if (sa->act.completion >= gcfg.actions[sa->act.type].completed_at - 1) {
		build_building(sim, &sa->act.range.center, bldg_house);

		return rs_done;
	} else if (sa->resources >= gcfg.blueprints[bldg_house].cost) {
		return rs_done;
	} else if (e->holding == et_resource_wood) {
		return deliver_resources(sim, e, sa);
	} else {
		return pickup_resources(sim, e, sa);
	}
}
