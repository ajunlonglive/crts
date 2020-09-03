#include "posix.h"

#ifndef CRTS_SERVER
#define CRTS_SERVER
#endif

#include "server/sim/action.h"
#include "server/sim/do_action.h"
#include "server/sim/do_action/carry.h"
#include "server/sim/pathfind/pathfind.h"
#include "server/sim/storehouse.h"
#include "shared/constants/globals.h"
#include "shared/sim/ent.h"
#include "shared/sim/tiles.h"
#include "shared/types/result.h"
#include "shared/util/log.h"

static enum result
dropoff_resources(struct simulation *sim, struct ent *e, struct point *p)
{
	enum result r;

	if (e->pg->unset) {
		struct storehouse_storage *st =
			nearest_storehouse(&sim->world->chunks, &e->pos,
				e->holding);
		L("found st with cap @ %d, %d", st->pos.x, st->pos.y);
		struct point rp;
		if (st && find_adj_tile(&sim->world->chunks, &st->pos, &rp,
			NULL, -1, e->trav, NULL, tile_is_traversable)) {
			ent_pgraph_set(e, &rp);
		} else {
			return rs_fail;
		}
	}

	switch (r = ent_pathfind(e)) {
	case rs_cont:
		break;
	case rs_fail:
	/* set_tile_inacessable(&act->hash, &act->local->goal); */
	/* FALLTHROUGH */
	case rs_done:
		L("arrived at storehouse");
		e->pg->unset = true;
		struct point rp;

		if (!find_adj_tile(&sim->world->chunks, &e->pos, &rp, NULL,
			tile_storehouse, 0, NULL, NULL)) {
			L("no longer there");
			return rs_cont;
		}

		struct storehouse_storage *st =
			get_storehouse_storage_at(&sim->world->chunks, &rp);

		if (!storehouse_store(st, e->holding)) {
			return rs_cont;
		}

		for (uint32_t i = 0; i < STOREHOUSE_SLOTS; ++i) {
			L("-> %s / %d",  gcfg.ents[st->type[i]].name, st->amnt[i]);
		}
		L("dropped off!");

		e->holding = et_none;
		break;
	}

	return r;
}

enum result
do_action_carry(struct simulation *sim, struct ent *e, struct sim_action *sa)
{
	if (sa->act.workers_waiting >= sa->act.workers_assigned) {
		return rs_done; /* all ents  drop their item when unassigned */
	} else if (e->state & es_waiting) {
		return rs_cont;
	}

	if (e->holding) {
		L("dropping off %s", gcfg.ents[e->holding].name);
		switch (dropoff_resources(sim, e, &sa->act.range.pos)) {
		case rs_cont:
			break;
		case rs_done:
			/* e->state |= es_waiting; */
			/* ++sa->act.workers_waiting; */
			break;
		case rs_fail:
			return rs_fail;
		}
	} else {
		switch (pickup_resources(sim, e, 0, &sa->act.range)) {
		case rs_cont:
		case rs_done:
			break;
		case rs_fail:
			e->state |= es_waiting;
			++sa->act.workers_waiting;
			break;
		}
	}

	return rs_cont;
}
