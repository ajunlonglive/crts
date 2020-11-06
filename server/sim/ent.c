#include "posix.h"

#ifndef CRTS_SERVER
#define CRTS_SERVER
#endif

#include <stdlib.h>

#include "server/sim/do_action.h"
#include "server/sim/ent.h"
#include "server/sim/worker.h"
#include "shared/constants/globals.h"
#include "shared/math/rand.h"
#include "shared/pathfind/meander.h"
#include "shared/pathfind/pathfind.h"
#include "shared/sim/ent.h"
#include "shared/sim/tiles.h"
#include "shared/types/result.h"
#include "shared/util/log.h"
#include "tracy.h"

void
drop_held_ent(struct world *w, struct ent *e)
{
	struct ent *drop;

	if (!e->holding) {
		return;
	}

	drop = spawn_ent(w);
	drop->pos = e->pos;
	drop->type = e->holding;

	e->holding = 0;
}

void
destroy_ent(struct world *w, struct ent *e)
{
	if (!(e->state & es_killed)) {
		darr_push(w->graveyard, &e->id);

		e->state |= (es_killed | es_modified);

		if (e->elctx) {
			ent_lookup_teardown(e->elctx);
		}
	}
}

void
kill_ent(struct simulation *sim, struct ent *e)
{
	struct ent *te;
	struct sim_action *sa;

	if (!(e->state & es_killed)) {
		if (e->state & es_have_task && (sa = action_get(sim, e->task))) {
			// TODO @performance: is this necessary? all it does is
			// call drop_held_ent which we already do for the other
			// branch, and decrement act->workers_assigned
			worker_unassign(sim, e, &sa->act);
		} else {
			drop_held_ent(sim->world, e);
		}

		destroy_ent(sim->world, e);

		if (gcfg.ents[e->type].corpse) {
			te = spawn_ent(sim->world);
			te->pos = e->pos;
			te->type = gcfg.ents[e->type].corpse;
		}
	}
}

void
damage_ent(struct simulation *sim, struct ent *e, uint8_t damage)
{
	/* TODO: check for overflow */
	if ((e->damage += damage) > gcfg.ents[e->type].hp) {
		kill_ent(sim, e);
	}
}

struct ent *
spawn_ent(struct world *w)
{
	struct ent *e = darr_get_mem(w->spawn);
	ent_init(e);
	e->id = w->seq++;

	return e;
}

enum iteration_result
process_spawn_iterator(void *_s, void *_e)
{
	struct ent *ne, *e = _e;
	struct simulation *s = _s;

	hdarr_set(s->world->ents, &e->id, e);
	ne = hdarr_get(s->world->ents, &e->id);
	ne->state = es_modified | es_spawned;
	ne->trav = gcfg.ents[ne->type].trav;

	if (gcfg.ents[ne->type].animate) {
		ne->elctx = calloc(1, sizeof(struct ent_lookup_ctx));
		ne->elctx->pg = calloc(1, sizeof(struct pgraph));
		pgraph_init(ne->elctx->pg, &s->world->chunks);
		ent_lookup_setup(ne->elctx);
	}

	return ir_cont;
}

static bool
process_hunger(struct simulation *sim, struct ent *e)
{
	if (e->state & es_hungry) {
		if (e->hunger) {
			if ((--e->hunger) & 8) {
				return true;
			}
		} else {
			switch (pickup_resources(sim, e, et_resource_crop, NULL)) {
			case rs_done:
				e->holding = et_none;
				e->state &= ~es_hungry;
				return false;
			case rs_cont:
				break;
			case rs_fail:
				damage_ent(sim, e, 1);
				e->hunger = gcfg.misc.food_search_cooldown;
				break;
			}
			return true;
		}
	} else if (e->type == et_worker && e->hunger >= gcfg.misc.max_hunger) {
		e->state |= es_hungry;
		e->hunger = 0;
	} else {
		++e->hunger;
	}

	return false;
}

static void
process_idle(struct simulation *sim, struct ent *e)
{
	TracyCZoneAutoS;
	if (rand_chance(gcfg.misc.meander_chance)) {
		meander(&sim->world->chunks, &e->pos, e->trav);
		e->state |= es_modified;
	}
	TracyCZoneAutoE;
}

enum iteration_result
simulate_ent(void *_sim, void *_e)
{
	TracyCZoneAutoS;
	struct simulation *sim = _sim;
	struct ent *e = _e;
	struct sim_action *sact;
	uint32_t over_age;

	if (get_tile_at(&sim->world->chunks, &e->pos) == tile_burning) {
		damage_ent(sim, e, gcfg.misc.fire_damage);
	}

	if (e->state & es_killed || gcfg.ents[e->type].phantom) {
		goto return_continue;
	} else if (!gcfg.ents[e->type].animate) {
		goto sim_age;
	}

	if (process_hunger(sim, e)) {
		goto sim_age;
	}

	if (!(e->state & es_have_task)) {
		process_idle(sim, e);
	} else {
		if ((sact = action_get(sim, e->task)) == NULL) {
			worker_unassign(sim, e, NULL);
		} else if (sact->act.completion >= gcfg.actions[sact->act.type].completed_at) {
			worker_unassign(sim, e, &sact->act);
		} else {
			switch (do_action(sim, e, sact)) {
			case rs_done:
				sact->act.completion++;
				break;
			case rs_fail:
				L("action %d failed", sact->act.id);
				action_complete(sim, sact->act.id);
				break;
			case rs_cont:
				break;
			}
		}
	}

sim_age:
	if (gcfg.ents[e->type].lifespan
	    && ++e->age >= gcfg.ents[e->type].lifespan) {
		if (gcfg.ents[e->type].animate) {
			over_age = ++e->age - gcfg.ents[e->type].lifespan;

			if (over_age < gcfg.misc.max_over_age
			    && (rand_chance(gcfg.misc.max_over_age - over_age))) {
				goto return_continue;
			}
		}

		kill_ent(sim, e);
	}

return_continue:
	TracyCZoneAutoE;

	return ir_cont;
}

bool
ent_pgraph_set(struct chunks *cnks, struct ent *e, const struct point *g)
{
	if (hpa_start(cnks, &e->pos, g, &e->path)) {
		e->state |= es_pathfinding;
		return true;
	}

	return false;
}

enum result
ent_pathfind(struct chunks *cnks, struct ent *e)
{
	enum result r;

	switch (r = hpa_continue(cnks, &e->path, &e->pos)) {
	case rs_cont:
		e->state |= es_modified;
		break;
	case rs_fail:
	case rs_done:
		break;
	}

	return r;
}
