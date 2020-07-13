#include "posix.h"

#include <assert.h>

#ifndef CRTS_SERVER
#define CRTS_SERVER
#endif

#include "server/sim/action.h"
#include "server/sim/do_action.h"
#include "server/sim/do_action/build.h"
#include "server/sim/ent.h"
#include "server/sim/pathfind/pathfind.h"
#include "server/sim/terrain.h"
#include "shared/constants/globals.h"
#include "shared/net/msg_queue.h"
#include "shared/types/result.h"
#include "shared/util/log.h"

//#define TGT_BLOCK blueprints[sa->act.tgt].blocks[e->subtask]
#define TGT_TILE gcfg.tiles[sa->act.tgt]

enum build_state {
	bs_not_built,
	bs_built,
	bs_building,
};

struct action_build_ctx {
	uint32_t tick;
	struct hash *failed_nav;
	struct hash *built;
	uint32_t built_count;
	bool initialized;
};

_Static_assert(sizeof(struct action_build_ctx) <= SIM_ACTION_CTX_LEN,
	"struct action_build_ctx too big");

struct reposition_ents_ctx {
	struct simulation *sim;
	struct point *p;
};

struct point
index_to_point(uint32_t index, const struct rectangle *r)
{
	struct point p = {
		.x = index % r->width,
		.y = index / r->width,
	};

	return point_add(&p, &r->pos);
}

uint32_t
point_to_index(const struct point *p, const struct rectangle *r)
{
	struct point q = point_sub(p, &r->pos);
	return q.x + q.y * r->width;
}

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

		if (!is_traversable(ctx->sim->world->chunks, &e->pos, e->trav)) {
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
deliver_resources(struct simulation *sim, struct ent *e, struct sim_action *sa,
	struct hash *reject)
{
	struct point p, q;
	enum result r;

	if (e->pg->unset) {
		q = index_to_point(e->subtask, &sa->act.range);
		//L("delivering to %d, %d", q.x, q.y);

		/* TODO: put this logic into find_adj_tile, refactoring the
		 * latter with a better interface */
		struct point adj[4] = {
			{ q.x + 1, q.y     },
			{ q.x - 1, q.y     },
			{ q.x,     q.y + 1 },
			{ q.x,     q.y - 1 },
		};

		uint8_t i, urej[4] = { 0 };
		const size_t *ret;
		for (i = 0; i < 4; ++i) {
			if ((ret = hash_get(reject, &adj[i]))) {
				urej[i] = 1;
			}
		}

		if (find_adj_tile(sim->world->chunks, &q, &p, NULL, -1,
			e->trav, urej, tile_is_traversable)) {
			ent_pgraph_set(e, &p);

			/* TODO: allow other ents a chance to do this before failing */
			hash_set(reject, &p, 1);
		} else {
			return rs_fail;
		}
	}

	switch (r = ent_pathfind(e)) {
	case rs_done:
		q = index_to_point(e->subtask, &sa->act.range);

		e->holding = et_none;

		if (TGT_TILE.functional) {
			update_functional_tile(sim->world->chunks, &q,
				sa->act.tgt, sa->act.motivator, 0);
		} else {
			update_tile(sim->world->chunks, &q, sa->act.tgt);
		}

		struct reposition_ents_ctx ctx = { sim, &q };

		hdarr_for_each(sim->world->ents, &ctx, reposition_ents);
		e->pg->unset = true;
		break;
	case rs_fail:
		e->pg->unset = true;
		break;
	case rs_cont:
		break;
	}

	return r == rs_done ? r : rs_cont;
}

enum result
do_action_build(struct simulation *sim, struct ent *e, struct sim_action *sa)
{
	const size_t *sp;
	struct action_build_ctx *ctx = (struct action_build_ctx *)sa->ctx;
	struct point p;

	if (!ctx->initialized) {
		ctx->failed_nav = hash_init(2048, 1, sizeof(struct point));
		ctx->built = hash_init(2048, 1, sizeof(struct point));
		ctx->initialized = true;
	}

	/* Handle dispatched ent */
	if (e->state & es_have_subtask) {
		p = index_to_point(e->subtask, &sa->act.range);

		if (e->holding || !TGT_TILE.makeup) {
			switch (deliver_resources(sim, e, sa, ctx->failed_nav)) {
			case rs_cont:
				break;
			case rs_fail:
				L("failed to deliver resource");
			/* Do the same thing as rs_done, don't cancel the whole
			 * action, allow buildings to be partially built.
			 */
			/* FALLTHROUGH */
			case rs_done:
				p = index_to_point(e->subtask, &sa->act.range);
				hash_set(ctx->built, &p, bs_built);
				e->state &= ~es_have_subtask;
			}

			return rs_cont;
		} else {
			switch (pickup_resources(sim, e, TGT_TILE.makeup, NULL)) {
			case rs_cont:
			case rs_done:
				break;
			case rs_fail:
				L("failed to pick up resource ");
				return rs_fail;
			}

			return rs_cont;
		}
	}

	enum blueprint blpt = gcfg.tiles[sa->act.tgt].build;
	uint64_t to_build;

	switch (blpt) {
	case blpt_none:
		return rs_fail;
		break;
	case blpt_single:
		to_build = 1;
		break;
	case blpt_frame:
		to_build = sa->act.range.width * 2 + sa->act.range.height * 2;
		break;
	case blpt_rect:
		to_build = sa->act.range.width * sa->act.range.height;
		break;
	}

	/* dispatch */
	ctx->built_count = 0;
	for (p.y = sa->act.range.pos.y;
	     p.y < sa->act.range.pos.y + (int64_t)sa->act.range.height;
	     ++p.y) {
		for (p.x = sa->act.range.pos.x;
		     p.x < sa->act.range.pos.x + (int64_t)sa->act.range.width;
		     ++p.x) {
			L("checking %d, %d, %d, %d", p.x, p.y, e->id, blpt);
			if ((sp = hash_get(ctx->built, &p)) && *sp) {
				if (*sp == bs_built) {
					ctx->built_count++;
				}

				goto check_next_block;
			} else if (!gcfg.tiles[get_tile_at(sim->world->chunks, &p)].foundation) {
				hash_set(ctx->built, &p, bs_built);
				goto check_next_block;
			}

			e->subtask = point_to_index(&p, &sa->act.range);
			e->state |= es_have_subtask;
			hash_set(ctx->built, &p, bs_building);

			return rs_cont;
check_next_block:
			switch (blpt) {
			case blpt_none:
				assert(false);
			case blpt_single:
				goto end_of_dispatch;
			case blpt_frame:
				if (p.x == sa->act.range.pos.x && p.y != sa->act.range.pos.y
				    && p.y != sa->act.range.pos.y + sa->act.range.height - 1
				    && sa->act.range.width > 2) {
					p.x += sa->act.range.width - 2;
				}
				break;
			case blpt_rect:
				break;
			}
		}
	}

end_of_dispatch:
	if (ctx->built_count >= to_build) {
		hash_destroy(ctx->failed_nav);
		hash_destroy(ctx->built);
		return rs_done;
	} else {
		/* If we made it this far, all blocks in the blueprint are accounted for
		 * so the current ent won't do anything this tick.
		 */
		return rs_cont;
	}
}
