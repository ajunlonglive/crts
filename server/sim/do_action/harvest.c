#ifndef CRTS_SERVER
#define CRTS_SERVER
#endif

#include "server/sim/action.h"
#include "server/sim/do_action.h"
#include "server/sim/do_action/harvest.h"
#include "server/sim/terrain.h"
#include "shared/constants/globals.h"
#include "shared/types/result.h"
#include "shared/util/log.h"

static enum tile harvest_target_to_tile[action_harvest_targets_count] = {
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
	struct point np, nnp;

	if (act->local == NULL) {
		if (find_tile(tgt, sim->world->chunks, &act->act.range, &e->pos,
			&np, act->hash)) {
			if (find_adj_tile(sim->world->chunks, &np, &nnp, NULL,
				-1, tile_is_traversable)) {
				act->local = pgraph_create(sim->world->chunks, &nnp);
			} else {
				set_tile_inacessable(&act->hash, &np);
				return rs_cont;
			}
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
	struct chunk *ck;
	struct point p, rp;
	struct ent *drop;
	uint8_t *harv;
	enum tile tgt_tile = harvest_target_to_tile[act->act.tgt];

	if (find_adj_tile(sim->world->chunks, &e->pos, &p, &act->act.range, tgt_tile, NULL)) {
		ck = get_chunk_at(sim->world->chunks, &p);
		rp = point_sub(&p, &ck->pos);
		harv = &ck->harvested[rp.x][rp.y];
		(*harv)++;
	} else {
		return goto_tile(sim, e, act, tgt_tile);
	}

	if (*harv >= gcfg.harvestable[act->act.tgt].diff) {
		*harv = 0;

		/* TODO: we shouldn't world_spawn inside an action, since it
		 * could case a realloc of the ent array while we are iterating
		 * through it... */
		drop = world_spawn(sim->world);
		drop->pos = p;
		drop->type = gcfg.harvestable[act->act.tgt].drop;

		update_tile(sim->world->chunks, &p,
			gcfg.harvestable[act->act.tgt].base);

		drop->changed = true;

		return rs_cont;
	} else {
		return rs_cont;
	}
}
