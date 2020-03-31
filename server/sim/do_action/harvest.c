#ifndef CRTS_SERVER
#define CRTS_SERVER
#endif

#include "server/sim/action.h"
#include "server/sim/do_action.h"
#include "server/sim/do_action/harvest.h"
#include "server/sim/pathfind/pathfind.h"
#include "server/sim/terrain.h"
#include "server/sim/worker.h"
#include "shared/constants/globals.h"
#include "shared/types/result.h"
#include "shared/util/log.h"

struct action_harvest_ctx {
	struct pgraph *pg;
};

_Static_assert(sizeof(struct action_harvest_ctx) <= SIM_ACTION_CTX_LEN,
	"struct action_harvest_ctx too big");

static void
set_tile_inacessable(struct hash **h, struct point *p)
{
	if (*h == NULL) {
		*h = hash_init(32, 1, sizeof(struct point));
	}

	hash_set(*h, p, 1);
}

static enum result
goto_tile(struct simulation *sim, struct ent *e, struct sim_action *act, enum tile tgt)
{
	struct point np, nnp;

	struct action_harvest_ctx *ctx = (void *)act->ctx;

	if (!ctx->pg || ctx->pg->unset) {
		if (find_tile(tgt, sim->world->chunks, &act->act.range, &e->pos,
			&np, act->hash)) {
			if (find_adj_tile(sim->world->chunks, &np, &nnp, NULL,
				-1, e->type, tile_is_traversable)) {

				if (!ctx->pg) {
					ctx->pg = pgraph_create(sim->world->chunks, &nnp, e->type);
				} else {
					pgraph_set(ctx->pg, &nnp, e->type);
				}
			} else {
				set_tile_inacessable(&act->hash, &np);
				return rs_cont;
			}
		} else {
			if (ctx->pg) {
				pgraph_destroy(ctx->pg);
			}

			return rs_done;
		}
	}

	switch (pathfind(ctx->pg, &e->pos)) {
	case rs_cont:
		e->state |= es_modified;
		break;
	case rs_fail:
		action_ent_blacklist(act, e);
		worker_unassign(sim, e, &act->act);
	/* FALLTHROUGH */
	case rs_done:
		ctx->pg->unset = true;
		break;
	}

	return rs_cont;
}

enum result
do_action_harvest(struct simulation *sim, struct ent *e, struct sim_action *act)
{
	struct chunk *ck;
	struct point p, rp;
	uint16_t *harv;
	enum tile tgt_tile = act->act.tgt;

	if (find_adj_tile(sim->world->chunks, &e->pos, &p, &act->act.range, tgt_tile, -1, NULL)) {
		ck = get_chunk_at(sim->world->chunks, &p);
		rp = point_sub(&p, &ck->pos);
		harv = &ck->harvested[rp.x][rp.y];
		(*harv)++;
	} else {
		return goto_tile(sim, e, act, tgt_tile);
	}

	if (*harv >= gcfg.tiles[act->act.tgt].hardness) {
		destroy_tile(sim, &p);
		return rs_cont;
	} else {
		return rs_cont;
	}
}
