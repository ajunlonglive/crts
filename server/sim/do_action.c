#include "server/sim/action.h"
#include "server/sim/do_action.h"
#include "server/sim/pathfind/pathfind.h"
#include "server/sim/pathfind/pgraph.h"
#include "server/sim/sim.h"
#include "server/sim/terrain.h"
#include "shared/messaging/server_message.h"
#include "shared/sim/ent.h"
#include "shared/util/log.h"

static enum action_result
do_action_harvest(struct simulation *sim, struct ent *e, struct sim_action *act)
{
	struct point np = nearest_chunk(&e->pos), rp = point_sub(&e->pos, &np);
	struct chunk *chnk = get_chunk(sim->world->chunks, &np);

	enum tile *cur_tile = &chnk->tiles[rp.x][rp.y];
	uint8_t *harv = &chnk->harvested[rp.x][rp.y];

	switch (*cur_tile) {
	case tile_forest:
		(*harv)++;
		break;
	default:
		if (act->local != NULL && !points_equal(&act->local->goal, &e->pos)) {
			if (pathfind_and_update(sim, act->local, e) != pr_cont) {
				L("destroying pgraph");
				pgraph_destroy(act->local);
				act->local = NULL;
			}
		} else if (find_tile(tile_forest, sim->world->chunks, &act->act.range, &np)) {
			L("creating pgraph, %d, %d", np.x, np.y);
			act->local = pgraph_create(sim->world->chunks, &np);
		} else {
			L("failed to find available tile");
		}

		return 0;
	}

	if (*harv > 100) {
		*harv = 0;
		*cur_tile = tile_plain;
		queue_push(sim->outbound, sm_create(server_message_chunk, chnk));
		return 1;
	} else {
		return 0;
	}
}

int
do_action(struct simulation *sim, struct ent *e, struct sim_action *act)
{
	switch (act->act.type) {
	case at_harvest:
		return do_action_harvest(sim, e, act);
	default:
		return 1;
	}
}
