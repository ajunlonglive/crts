#ifndef CRTS_SERVER
#define CRTS_SERVER
#endif

#include "server/sim/action.h"
#include "server/sim/do_action.h"
#include "server/sim/do_action/build.h"
#include "shared/constants/globals.h"
#include "shared/types/result.h"
#include "shared/util/log.h"

static void
create_building(struct simulation *sim, struct point *center)
{
	struct point p = *center;

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
}

static enum result
deliver_resources(struct simulation *sim, struct ent *e, struct sim_action *sa)
{
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
		return rs_fail;
	}

	return rs_cont;
}

static enum result
pickup_resources(struct simulation *sim, struct ent *e, struct sim_action *sa)
{
	struct ent *wood;

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
		return rs_fail;
	}

	return rs_cont;
}

enum result
do_action_build(struct simulation *sim, struct ent *e, struct sim_action *sa)
{
	if (sa->act.completion >= gcfg.actions[sa->act.type].completed_at - 1) {
		create_building(sim, &sa->act.range.center);

		return rs_done;
	} else if (sa->resources >= 15) {
		return rs_done;
	} else if (e->holding == et_resource_wood) {
		return deliver_resources(sim, e, sa);
	} else {
		return pickup_resources(sim, e, sa);
	}
}
