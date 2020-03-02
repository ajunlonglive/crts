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
#include "shared/types/result.h"
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

	return *et == e->type;
}

static struct ent *
find_resource(struct world *w, enum ent_type t, struct point *p)
{
	return find_ent(w, p, &t, find_resource_pred);

	return false;
}

static enum result
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
		    && pathfind_and_update(sim, act->local, e) != rs_cont) {
			pgraph_destroy(act->local);
			act->local = NULL;
		} else if (find_tile(tile_forest, sim->world->chunks, &act->act.range, &np)) {
			if (act->local != NULL) {
				pgraph_destroy(act->local);
			}

			act->local = pgraph_create(sim->world->chunks, &np);
		} else {
			return rs_done;
		}

		return rs_cont;
	}

	if (*harv > 100) {
		*harv = 0;
		*cur_tile = tile_plain;
		w = world_spawn(sim->world);
		w->pos = e->pos;
		w->type = et_resource_wood;

		queue_push(sim->outbound, sm_create(server_message_ent, w));
		queue_push(sim->outbound, sm_create(server_message_chunk, chnk));

		return rs_cont;
	} else {
		return rs_cont;
	}
}

enum result
do_action_build(struct simulation *sim, struct ent *e, struct sim_action *sa)
{
	struct ent *wood;
	struct point p;
	enum result ar = rs_cont;
	if (sa->act.completion >= gcfg.actions[sa->act.type].completed_at - 1) {
		L("we are done building!");
		p = sa->act.range.center;

		update_tile(sim, &p, tile_bldg);

		p.x++;
		update_tile(sim, &p, tile_bldg);

		p.x -= 2;
		update_tile(sim, &p, tile_bldg);

		p.x++;
		p.y++;
		update_tile(sim, &p, tile_bldg);

		p.y -= 2;
		update_tile(sim, &p, tile_bldg);

		ar = rs_done;
	} else if (sa->resources >= 15) {
		ar = rs_done;
	} else if (e->holding == et_resource_wood) {
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
			ar = rs_fail;
			break;
		}
	} else {
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
			ar = rs_fail;
			break;
		}
	}

	return ar;
}

enum result
do_action_move(struct simulation *sim, struct ent *e, struct sim_action *sa)
{
	if (e->wait) {
		if (sa->act.workers_waiting >= sa->act.workers_assigned) {
			return rs_done;
		} else {
			return rs_cont;
		}
	}

	switch (pathfind_and_update(sim, sa->global, e)) {
	case rs_cont:
		break;
	case rs_fail:
		return rs_fail;
		break;
	case rs_done:
		e->wait = true;
		sa->act.workers_waiting++;
		break;
	}

	return rs_cont;
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
	default:
		return rs_done;
	}
}
