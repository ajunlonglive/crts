#ifndef CRTS_SERVER
#define CRTS_SERVER
#endif

#include "server/sim/action.h"
#include "server/sim/do_action.h"
#include "server/sim/do_action/build.h"
#include "server/sim/terrain.h"
#include "shared/constants/globals.h"
#include "shared/net/msg_queue.h"
#include "shared/types/result.h"
#include "shared/util/log.h"

#define TGT_BLOCK blueprints[sa->act.tgt].blocks[e->subtask]
#define TGT_TILE gcfg.tiles[TGT_BLOCK.t]

struct action_build_ctx {
	uint32_t built;
	uint32_t dispatched;
	uint32_t counted;
	uint32_t tick;
};

_Static_assert(sizeof(struct action_build_ctx) <= SIM_ACTION_CTX_LEN,
	"struct action_build_ctx too big");

struct reposition_ents_ctx {
	struct simulation *sim;
	struct point *p;
};

static enum iteration_result
reposition_ents(void *_ctx, void *_e)
{
	struct ent *e = _e;
	struct reposition_ents_ctx *ctx = _ctx;
	bool repos, didrepos = false;

	if (!points_equal(&e->pos, ctx->p)) {
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
		e->state |= es_modified;
	}

	return ir_cont;
}

static enum result
deliver_resources(struct simulation *sim, struct ent *e, struct sim_action *sa)
{
	struct point p, q;
	enum result r;

	if (e->pg == NULL) {
		q = point_add(&TGT_BLOCK.p, &sa->act.range.center);

		if (find_adj_tile(sim->world->chunks, &q, &p, NULL, -1,
			tile_is_traversable)) {
			e->pg = pgraph_create(sim->world->chunks, &p);
		} else {
			return rs_fail;
		}

	}

	switch (r = pathfind_and_update(sim, e->pg, e)) {
	case rs_done:
		q = point_add(&TGT_BLOCK.p, &sa->act.range.center);

		e->holding = et_none;

		update_tile(sim->world->chunks, &q, TGT_BLOCK.t);

		struct reposition_ents_ctx ctx = { sim, &q };

		hdarr_for_each(sim->world->ents, &ctx, reposition_ents);
	/* FALLTHROUGH */
	case rs_fail:
		/* TODO: ents pathgraph should be created once, and then use
		 * pgraph_reset when we want to delete it.
		 */
		pgraph_destroy(e->pg);
		e->pg = NULL;
		break;
	case rs_cont:
		break;
	}

	/* If we failed to pathfind, then try again with a new goal.  This
	 * could happen if something got built at our old goal.
	 */
	return r == rs_done ? r : rs_cont;
}

static enum result
pickup_resources(struct simulation *sim, struct ent *e, struct sim_action *sa)
{
	struct ent *res;
	enum result r;

	if (e->pg == NULL) {
		if ((res = find_resource(sim->world, TGT_TILE.makeup, &e->pos)) != NULL) {
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
		    && points_equal(&e->pos, &res->pos)) {
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
do_action_build(struct simulation *sim, struct ent *e, struct sim_action *sa)
{
	size_t i;
	uint32_t j;
	struct action_build_ctx *ctx = (struct action_build_ctx *)sa->ctx;

	/* If we have built all blocks, we are done */
	if (ctx->built == blueprints[sa->act.tgt].len) {
		return rs_done;
	}

	/* Every tick, we have to make sure that all dispatched ents are still
	 * around, so we keep track of which ones we saw in counted.
	 */
	if (ctx->tick != sim->tick) {
		ctx->tick = sim->tick;
		ctx->dispatched = ctx->counted;
		ctx->counted = 0;
	}

	/* Handle dispatched ent */
	if (!(e->state & es_have_subtask)) {
		ctx->counted |= 1 << e->subtask;

		if (e->holding) {
			switch (deliver_resources(sim, e, sa)) {
			case rs_cont:
				break;
			case rs_fail:
				L("failed to deliver resource");
			/* Do the same thing as done, don't cancel the whole
			 * action, allow buildings to be partially built.
			 */
			/* FALLTHROUGH */
			case rs_done:
				ctx->built |= 1 << e->subtask;
				e->state |= es_have_subtask;
			}

			return rs_cont;
		} else {
			switch (pickup_resources(sim, e, sa)) {
			case rs_cont:
			case rs_done:
				break;
			case rs_fail:
				L("failed to pick up resource");
				return rs_fail;
			}
		}
	}

	/* dispatch */
	for (i = 0; i < BLUEPRINT_LEN; ++i) {
		j = 1 << i;

		if (!(blueprints[sa->act.tgt].len & j)) {
			break;
		} else if (ctx->built & j || ctx->dispatched & j) {
			continue;
		}

		ctx->counted |= j;
		ctx->dispatched |= j;
		e->subtask = i;
		e->state &= ~es_have_subtask;
		return rs_cont;
	}

	/* If we made it this far, all blocks in the blueprint are accounted for
	 * so the current ent won't do anything this tick.
	 */
	return rs_cont;
}
