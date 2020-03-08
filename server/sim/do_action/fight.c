#ifndef CRTS_SERVER
#define CRTS_SERVER
#endif

#include "server/sim/action.h"
#include "server/sim/do_action/fight.h"
#include "shared/sim/alignment.h"
#include "shared/sim/ent.h"
#include "shared/types/result.h"
#include "shared/util/log.h"

static bool
find_enemy_pred(void *_pe, struct ent *e)
{
	struct ent *pe = _pe;

	return e->type == et_worker && e->alignment->max != pe->alignment->max;
}

enum result
do_action_fight(struct simulation *sim, struct ent *e, struct sim_action *sa)
{
	struct ent *en;

	if (e->pg == NULL) {
		if ((en = find_ent(sim->world, &e->pos, e, find_enemy_pred)) != NULL) {
			e->pg = pgraph_create(sim->world->chunks, &en->pos);
			e->target = en->id;
		} else {
			return rs_fail;
		}
	}

	switch (pathfind_and_update(sim, e->pg, e)) {
	case rs_fail:
		pgraph_destroy(e->pg);
		e->pg = NULL;
		return rs_fail;
	case rs_done:
		if ((en = hdarr_get(sim->world->ents, &e->target)) != NULL) {
			if (points_equal(&e->pos, &en->pos)) {
				if (++en->damage < 10) {
					return rs_cont;
				} else {
					kill_ent(sim, en);
				}
			}
		}

		pgraph_destroy(e->pg);
		e->pg = NULL;
		break;
	case rs_cont:
		break;
	}

	return rs_cont;
}
