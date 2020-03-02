#ifndef CRTS_SERVER
#define CRTS_SERVER
#endif

#define _XOPEN_SOURCE 500

#include "server/sim/action.h"
#include "server/sim/do_action.h"
#include "server/sim/do_action/build.h"
#include "server/sim/do_action/harvest.h"
#include "server/sim/do_action/move.h"
#include "server/sim/terrain.h"
#include "shared/messaging/server_message.h"
#include "shared/types/result.h"

void
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

struct ent *
find_resource(struct world *w, enum ent_type t, struct point *p)
{
	return find_ent(w, p, &t, find_resource_pred);

	return false;
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
