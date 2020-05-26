#ifndef CRTS_SERVER
#define CRTS_SERVER
#endif

#include <stdlib.h>

#include "server/sim/do_action.h"
#include "server/sim/ent.h"
#include "server/sim/pathfind/meander.h"
#include "server/sim/terrain.h"
#include "server/sim/worker.h"
#include "shared/constants/globals.h"
#include "shared/math/rand.h"
#include "shared/sim/ent.h"
#include "shared/types/result.h"
#include "shared/util/log.h"

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
kill_ent(struct simulation *sim, struct ent *e)
{
	struct ent *te;
	struct sim_action *sa;

	if (!(e->state & es_killed)) {
		darr_push(sim->world->graveyard, &e->id);

		e->state |= (es_killed | es_modified);

		if (e->state & es_have_task && (sa = action_get(sim, e->task))) {
			worker_unassign(sim, e, &sa->act);
		} else {
			drop_held_ent(sim->world, e);
		}

		if (e->pg) {
			pgraph_destroy(e->pg);
		}

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

	return e;
}

enum iteration_result
process_spawn_iterator(void *_s, void *_e)
{
	struct ent *ne, *e = _e;
	struct simulation *s = _s;

	ne = world_spawn(s->world);

	ne->type = e->type;
	ne->pos = e->pos;
	ne->holding = e->holding;
	ne->alignment = e->alignment;
	ne->state = es_modified;
	ne->trav = gcfg.ents[ne->type].trav;

	if (gcfg.ents[ne->type].animate) {
		ne->pg = calloc(1, sizeof(struct pgraph));
		pgraph_init(ne->pg, s->world->chunks);
	}

	return ir_cont;
}

void
mount_vehicle(struct ent *e, struct ent *ve)
{
	e->riding = ve->id;
	e->trav = gcfg.ents[ve->type].trav;
	e->pos = ve->pos;
	e->state |= es_modified;
}

void
unmount_vehicle(struct ent *e, struct ent *ve, struct point *up)
{
	e->riding = 0;
	e->trav = gcfg.ents[e->type].trav;
	e->pos = *up;
	e->state |= es_modified;
}

enum iteration_result
simulate_ent(void *_sim, void *_e)
{
	struct simulation *sim = _sim;
	struct ent *ve, *e = _e;
	struct sim_action *sact;
	uint32_t over_age;

	if (get_tile_at(sim->world->chunks, &e->pos) == tile_burning) {
		damage_ent(sim, e, gcfg.misc.fire_damage);
	}

	if (e->state & es_killed) {
		return ir_cont;
	} else if (!gcfg.ents[e->type].animate) {
		goto sim_age;
	}

	if (e->satisfaction > 0) {
		e->satisfaction--;
	}

	if (!(e->state & es_have_task)) {
		if (rand_chance(gcfg.misc.meander_chance)) {
			meander(sim->world->chunks, &e->pos, e->trav);
			e->state |= es_modified;
		}
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

	if (e->riding
	    && (ve = hdarr_get(sim->world->ents, &e->riding))
	    && !points_equal(&ve->pos, &e->pos)) {
		L("moving ride %d to my pos", e->riding);
		ve->pos = e->pos;
		ve->state |= es_modified;
	}

sim_age:
	if (gcfg.ents[e->type].lifespan
	    && ++e->age >= gcfg.ents[e->type].lifespan) {
		if (gcfg.ents[e->type].animate) {
			over_age = ++e->age - gcfg.ents[e->type].lifespan;

			if (over_age < gcfg.misc.max_over_age
			    && (rand_chance(gcfg.misc.max_over_age - over_age))) {
				return ir_cont;
			}
		}

		kill_ent(sim, e);
	}

	return ir_cont;
}
