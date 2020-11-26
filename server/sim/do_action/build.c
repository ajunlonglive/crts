#include "posix.h"

#include <assert.h>

#ifndef CRTS_SERVER
#define CRTS_SERVER
#endif

#include "server/sim/action.h"
#include "server/sim/do_action.h"
#include "server/sim/do_action/build.h"
#include "server/sim/ent.h"
#include "server/sim/update_tile.h"
#include "shared/constants/globals.h"
#include "shared/net/msg_queue.h"
#include "shared/sim/tiles.h"
#include "shared/types/result.h"
#include "shared/util/log.h"

//#define TGT_BLOCK blueprints[sa->act.tgt].blocks[e->subtask]
#define TGT_TILE gcfg.tiles[sa->act.tgt]

enum build_state {
	bs_not_built,
	bs_built,
};

struct action_build_ctx {
	struct hash failed_nav, built;
	uint32_t tick;
	uint32_t built_count;
};

_Static_assert(sizeof(struct action_build_ctx) <= SIM_ACTION_CTX_LEN,
	"struct action_build_ctx too big");

struct reposition_ents_ctx {
	struct simulation *sim;
	struct point *p;
};

void
index_to_point(uint32_t index, const struct rectangle *r, struct point *p)
{
	p->x = index % r->width + r->pos.x;
	p->y = index / r->width + r->pos.y;
}

uint32_t
point_to_index(const struct point *p, const struct rectangle *r)
{
	return (p->x - r->pos.x) + (p->y - r->pos.y) * r->width;
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

		if (!is_traversable(&ctx->sim->world->chunks, &e->pos, e->trav)) {
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

	if (!(e->state & es_pathfinding)) {
		index_to_point(e->subtask, &sa->act.range, &q);
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

		if (find_adj_tile(&sim->world->chunks, &q, &p, NULL, -1,
			e->trav, urej, tile_is_traversable)) {
			if (!ent_pgraph_set(&sim->world->chunks, e, &p)) {
				return rs_fail;
			}

			/* TODO: allow other ents a chance to do this before failing */
			hash_set(reject, &p, 1);
		} else {
			return rs_fail;
		}
	}

	switch (r = ent_pathfind(&sim->world->chunks, e)) {
	case rs_done:
		index_to_point(e->subtask, &sa->act.range, &q);

		e->holding = et_none;

		if (TGT_TILE.function) {
			update_functional_tile(sim->world, &q,
				sa->act.tgt, sa->act.motivator, 0);
		} else {
			update_tile(sim->world, &q, sa->act.tgt);
		}

		struct reposition_ents_ctx ctx = { sim, &q };

		hdarr_for_each(&sim->world->ents, &ctx, reposition_ents);
		break;
	case rs_fail:
		break;
	case rs_cont:
		break;
	}

	return r == rs_done ? r : rs_cont;
}

void
do_action_build_setup(struct simulation *sim, struct sim_action *sa)
{
	struct action_build_ctx *ctx = (struct action_build_ctx *)sa->ctx;

	hash_init(&ctx->failed_nav, 2048, sizeof(struct point));
	hash_init(&ctx->built, 2048, sizeof(struct point));
}

void
do_action_build_teardown(struct sim_action *sa)
{
	struct action_build_ctx *ctx = (struct action_build_ctx *)sa->ctx;
	hash_destroy(&ctx->failed_nav);
	hash_destroy(&ctx->built);
}

enum result
do_action_build(struct simulation *sim, struct ent *e, struct sim_action *sa)
{
	const size_t *sp;
	struct action_build_ctx *ctx = (struct action_build_ctx *)sa->ctx;
	struct point p;

	/* Handle dispatched ent */
	if (e->state & es_have_subtask) {
		index_to_point(e->subtask, &sa->act.range, &p);

		if (e->holding || !TGT_TILE.makeup) {
			switch (deliver_resources(sim, e, sa, &ctx->failed_nav)) {
			case rs_cont:
				break;
			case rs_fail:
				L("failed to deliver resource");
			/* Do the same thing as rs_done, don't cancel the whole
			 * action, allow buildings to be partially built.
			 */
			/* FALLTHROUGH */
			case rs_done:
				index_to_point(e->subtask, &sa->act.range, &p);
				hash_set(&ctx->built, &p, bs_built);
				e->state &= ~es_have_subtask;
			}

			return rs_cont;
		} else {
			switch (pickup_resources(sim, &sa->elctx, e, TGT_TILE.makeup, NULL)) {
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
	uint64_t to_build = 0;

	switch (blpt) {
	case blpt_none:
		return rs_fail;
		break;
	case blpt_single:
		to_build = 1;
		break;
	case blpt_frame:
		if (sa->act.range.width == 1) {
			to_build = sa->act.range.height;
		} else if (sa->act.range.height == 1) {
			to_build = sa->act.range.width;
		} else {
			to_build = ((sa->act.range.width + sa->act.range.height) * 2) - 4;
		}

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
			if ((sp = hash_get(&ctx->built, &p)) && *sp) {
				ent_id_t id = *sp - 1;
				struct ent *e2;

				if (*sp == bs_built) {
					ctx->built_count++;
				} else if (!(e2 = hdarr_get(&sim->world->ents, &id))
					   || e->task != sa->act.id) {
					hash_unset(&ctx->built, &p);
				}
			} else if (!gcfg.tiles[get_tile_at(&sim->world->chunks, &p)].foundation) {
				hash_set(&ctx->built, &p, bs_built);
			} else {
				e->subtask = point_to_index(&p, &sa->act.range);

				struct point q;
				index_to_point(e->subtask, &sa->act.range, &q);

				assert_m(points_equal(&q, &p), "(%d, %d) == (%d, %d)",
					p.x, p.y, q.x, q.y);

				e->state |= es_have_subtask;
				hash_set(&ctx->built, &p, e->id + 1);

				return rs_cont;
			}

			switch (blpt) {
			case blpt_none:
				assert(false);
				break;
			case blpt_single:
				goto end_of_dispatch;
			case blpt_frame:
				if (sa->act.range.width > 2
				    && p.x == sa->act.range.pos.x
				    && p.y != sa->act.range.pos.y
				    && p.y != sa->act.range.pos.y + sa->act.range.height - 1) {
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
		return rs_done;
	} else {
		/* If we made it this far, all blocks in the blueprint are accounted for
		 * so the current ent won't do anything this tick.
		 */
		return rs_cont;
	}
}
