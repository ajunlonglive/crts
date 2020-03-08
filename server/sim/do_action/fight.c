#ifndef CRTS_SERVER
#define CRTS_SERVER
#endif

#include "server/sim/action.h"
#include "server/sim/do_action/fight.h"
#include "shared/sim/alignment.h"
#include "shared/sim/ent.h"
#include "shared/types/result.h"
#include "shared/util/log.h"

struct find_enemy_pred_ctx {
};

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

	//L("ent: %d, target: %d", e->id, e->target);

	if (e->pg == NULL) {
		if ((en = find_ent(sim->world, &e->pos, e, find_enemy_pred)) != NULL) {
			e->pg = pgraph_create(sim->world->chunks, &en->pos);
			//L("no tgt, found %d", en->id);
			e->target = en->id;
		} else {
			L("couldnt' find enemy");
			return rs_fail;
		}
	}

	switch (pathfind_and_update(sim, e->pg, e)) {
	case rs_done:
		if ((en = hdarr_get(sim->world->ents, &e->target)) != NULL
		    && points_equal(&e->pos, &en->pos)) {
			L("fighting, me(%d) vs %d[%d]", e->id, en->id, en->damage);
			if (++en->damage < 10) {
				return rs_cont;
			}
		}

		sim_destroy_ent(sim, en);
		pgraph_destroy(e->pg);
		e->pg = NULL;
	case rs_cont:
		break;
	case rs_fail:
		L("couldnt' get to enemy");
		return rs_fail;
	}

	return rs_cont;
}
