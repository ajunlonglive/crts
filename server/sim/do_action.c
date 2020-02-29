#ifndef CRTS_SERVER
#define CRTS_SERVER
#endif

#define _XOPEN_SOURCE 500

#include "server/sim/action.h"
#include "server/sim/do_action.h"
#include "server/sim/pathfind/pathfind.h"
#include "server/sim/pathfind/pgraph.h"
#include "server/sim/sim.h"
#include "server/sim/terrain.h"
#include "shared/constants/globals.h"
#include "shared/messaging/server_message.h"
#include "shared/sim/ent.h"
#include "shared/util/log.h"

static void
update_tile(struct simulation *sim, const struct point *p, enum tile t)
{
	struct point rp, np = nearest_chunk(p);
	struct chunk *ck = get_chunk(sim->world->chunks, &np);

	rp = point_sub(p, &np);
	ck->tiles[rp.x][rp.y] = t;

	queue_push(sim->outbound, sm_create(server_message_chunk, ck));
}

static bool
find_resource_pred(void *ctx, struct ent *e)
{
	enum ent_type *et = ctx;
	L("checking ent %p (%d) for type %d", e, e->type, *et);

	return *et == e->type;
}

static struct ent *
find_resource(struct world *w, enum ent_type t, struct point *p)
{
	return find_ent(w, p, &t, find_resource_pred);

	return false;
}

static enum action_result
do_action_harvest(struct simulation *sim, struct ent *e, struct sim_action *act)
{
	struct point np = nearest_chunk(&e->pos), rp = point_sub(&e->pos, &np);
	struct chunk *chnk = get_chunk(sim->world->chunks, &np);
	struct ent *w;
	enum tile *cur_tile = &chnk->tiles[rp.x][rp.y];
	uint8_t *harv = &chnk->harvested[rp.x][rp.y];

	switch (*cur_tile) {
	case tile_forest:
		(*harv)++;
		break;
	default:
		if (act->local != NULL
		    && !points_equal(&act->local->goal, &e->pos)
		    && pathfind_and_update(sim, act->local, e) != pr_cont) {
			pgraph_destroy(act->local);
			act->local = NULL;
		} else if (find_tile(tile_forest, sim->world->chunks, &act->act.range, &np)) {
			if (act->local != NULL) {
				pgraph_destroy(act->local);
			}

			act->local = pgraph_create(sim->world->chunks, &np);
		} else {
			L("failed to find available tile");
			return ar_fail;
		}

		return ar_cont;
	}

	if (*harv > 100) {
		*harv = 0;
		*cur_tile = tile_plain;
		w = world_spawn(sim->world);
		w->pos = e->pos;
		w->type = et_resource_wood;

		queue_push(sim->outbound, sm_create(server_message_ent, w));
		queue_push(sim->outbound, sm_create(server_message_chunk, chnk));
		return ar_done;
	} else {
		return ar_cont;
	}
}

enum action_result
do_action_build(struct simulation *sim, struct ent *e, struct sim_action *sa)
{
	struct point p;

	if (find_ent(et_resource_wood, sim->world, &sa->act.range, &p)) {

	}

	return 0;
}

int
do_action(struct simulation *sim, struct ent *e, struct sim_action *act)
{
	switch (act->act.type) {
	case at_harvest:
		return do_action_harvest(sim, e, act);
	default:
		return ar_done;
	}
}
