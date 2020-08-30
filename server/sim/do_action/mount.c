#include "posix.h"

#ifndef CRTS_SERVER
#define CRTS_SERVER
#endif

#include "server/sim/do_action.h"
#include "server/sim/do_action/mount.h"
#include "server/sim/ent.h"
#include "server/sim/pathfind/pathfind.h"
#include "server/sim/terrain.h"
#include "shared/constants/globals.h"
#include "shared/sim/ent.h"
#include "shared/types/result.h"
#include "shared/util/log.h"

struct valid_vehicle_ctx {
	enum ent_type t;
	uint8_t trav_from;
	struct chunks *cnks;
	const struct rectangle *r;
};

static bool
valid_vehicle(void *_ctx, struct ent *e)
{
	struct valid_vehicle_ctx *ctx = _ctx;

	return e->type == ctx->t
	       && point_in_rect(&e->pos, ctx->r)
	       && find_adj_tile(ctx->cnks, &e->pos, NULL, NULL, -1,
		ctx->trav_from, NULL, tile_is_traversable);
}

static struct ent *
get_vehicle(struct world *w, const struct ent *e, enum ent_type tgt,
	const struct rectangle *r, struct point *ap)
{
	struct ent *ve;
	struct valid_vehicle_ctx ctx = { tgt, e->trav, &w->chunks, r };

	if (
		/* ent can ride vehicle */
		e->type == et_worker
		/* applicable vehicle found */
		&& (ve = find_ent(w, &e->pos, &ctx, valid_vehicle))) {

		find_adj_tile(&w->chunks, &ve->pos, ap, NULL, -1, ctx.trav_from,
			NULL, tile_is_traversable);

		return ve;
	} else {
		return NULL;
	}
}

enum result
do_action_mount(struct simulation *sim, struct ent *e, struct sim_action *sa)
{
	struct ent *ve;
	struct point ap;

	if (sa->act.workers_waiting >= sa->act.workers_assigned) {
		return rs_done;
	}

	if (!e->target) {
		if ((ve = get_vehicle(sim->world, e, et_vehicle_boat,
			&sa->act.range, &ap))) {
			L("found vehicle %d: %s, @ %d, %d, mount point: %d, %d",
				ve->id,
				gcfg.ents[ve->type].name, ve->pos.x, ve->pos.y,
				ap.x, ap.y);

			e->target = ve->id;
			ent_pgraph_set(e, &ap);
		} else {
			/* TODO: maybe just wait here? */
			return rs_fail;
		}
	} else {
		switch (ent_pathfind(e)) {
		case rs_done:
			if ((ve = hdarr_get(sim->world->ents, &e->target))) {
				mount_vehicle(e, ve);
				e->state |= es_waiting;
				++sa->act.workers_waiting;
			} else {
				e->target = 0;
			}
		/* FALLTHROUGH */
		case rs_fail:
			/* e->pg->unset = true; */
			break;
		case rs_cont:
			break;
		}
	}

	return rs_cont;
}
