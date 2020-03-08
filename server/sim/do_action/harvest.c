#ifndef CRTS_SERVER
#define CRTS_SERVER
#endif

#include "server/sim/action.h"
#include "server/sim/do_action/harvest.h"
#include "server/sim/terrain.h"
#include "shared/messaging/server_message.h"
#include "shared/types/result.h"
#include "shared/util/log.h"

enum tile harvest_target_to_tile[action_harvest_targets_count] = {
	[aht_forest]   = tile_forest,
	[aht_mountain] = tile_mountain,
	[aht_bldg]     = tile_bldg,
};

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
	struct point np;

	if (act->local == NULL) {
		if (find_tile(tgt, sim->world->chunks, &act->act.range, &e->pos,
			&np, act->hash)) {
			act->local = pgraph_create(sim->world->chunks, &np);
		} else {
			return rs_done;
		}
	}

	switch (pathfind_and_update(sim, act->local, e)) {
	case rs_cont:
		break;
	case rs_fail:
		set_tile_inacessable(&act->hash, &act->local->goal);
	/* FALLTHROUGH */
	case rs_done:
		pgraph_destroy(act->local);
		act->local = NULL;
		break;
	}

	return rs_cont;
}

enum result
do_action_harvest(struct simulation *sim, struct ent *e, struct sim_action *act)
{
	struct point np = nearest_chunk(&e->pos), rp = point_sub(&e->pos, &np);
	struct chunk *chnk = get_chunk(sim->world->chunks, &np);
	struct ent *w;
	enum tile tgt_tile, *cur_tile = &chnk->tiles[rp.x][rp.y];
	uint8_t *harv = &chnk->harvested[rp.x][rp.y];
	tgt_tile = harvest_target_to_tile[act->act.tgt];

	if (point_in_circle(&e->pos, &act->act.range) && *cur_tile == tgt_tile) {
		(*harv)++;
	} else {
		return goto_tile(sim, e, act, tgt_tile);
	}

	if (*harv > 1) {
		*harv = 0;

		w = world_spawn(sim->world);
		w->pos = e->pos;
		w->type = et_resource_wood;

		update_tile(sim->world->chunks, &e->pos, tile_plain);

		queue_push(sim->outbound, sm_create(server_message_ent, w));

		return rs_cont;
	} else {
		return rs_cont;
	}
}
