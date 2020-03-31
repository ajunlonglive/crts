#ifndef CRTS_SERVER
#define CRTS_SERVER
#endif

#define _XOPEN_SOURCE 500

#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>

#include "server/sim/action.h"
#include "server/sim/do_action.h"
#include "server/sim/environment.h"
#include "server/sim/pathfind/meander.h"
#include "server/sim/pathfind/pathfind.h"
#include "server/sim/sim.h"
#include "server/sim/terrain.h"
#include "server/sim/worker.h"
#include "shared/constants/globals.h"
#include "shared/sim/ent.h"
#include "shared/types/result.h"
#include "shared/util/log.h"

static struct point
get_valid_spawn(struct chunks *chunks, enum ent_type et)
{
	struct point p = {
		random() % gcfg.misc.initial_spawn_range,
		random() % gcfg.misc.initial_spawn_range
	}, q;
	const struct chunk *ck;
	int i, j;

	while (1) {
		p.x += CHUNK_SIZE;
		ck = get_chunk(chunks, &p);

		for (i = 0; i < CHUNK_SIZE; i++) {
			for (j = 0; j < CHUNK_SIZE; j++) {
				if (tile_is_traversable(ck->tiles[i][j], et)) {
					q.x = i; q.y = j;
					return point_add(&p, &q);
				}
			}
		}
	}
}

static void
populate(struct simulation *sim, uint16_t amnt, uint16_t algn)
{
	size_t i;
	struct ent *e;
	struct point p = get_valid_spawn(sim->world->chunks, et_worker);

	for (i = 0; i < amnt; i++) {
		e = spawn_ent(sim);
		e->type = et_worker;
		e->pos = p;
		e->alignment = algn;
	}
}

void
drop_held_ent(struct simulation *sim, struct ent *e)
{
	struct ent *drop;

	if (!e->holding) {
		return;
	}

	drop = spawn_ent(sim);
	drop->pos = e->pos;
	drop->type = e->holding;

	e->holding = 0;
}

void
destroy_tile(struct simulation *sim, struct point *p)
{
	struct ent *drop;
	enum tile t = get_tile_at(sim->world->chunks, p);

	drop = spawn_ent(sim);
	drop->pos = *p;
	drop->type = gcfg.tiles[t].drop;

	update_tile(sim->world->chunks, p, gcfg.tiles[t].base);
}

void
kill_ent(struct simulation *sim, struct ent *e)
{
	struct ent *te;
	struct sim_action *sa;

	if (!(e->state & es_killed)) {
		darr_push(sim->world->graveyard, &e->id);

		if (e->pg) {
			pgraph_destroy(e->pg);
		}

		e->state |= (es_killed | es_modified);

		if (e->state & es_have_task && (sa = action_get(sim, e->task))) {
			worker_unassign(sim, e, &sa->act);
		} else {
			drop_held_ent(sim, e);
		}

		if (gcfg.ents[e->type].corpse) {
			te = spawn_ent(sim);
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
spawn_ent(struct simulation *sim)
{
	struct ent *e = darr_get_mem(sim->world->spawn);
	ent_init(e);

	return e;
}

uint16_t
add_new_motivator(struct simulation *sim)
{
	uint16_t nm = ++sim->seq;

	populate(sim, gcfg.misc.initial_spawn_amount, nm);

	return nm;
}

struct simulation *
sim_init(struct world *w)
{
	struct simulation *sim = calloc(1, sizeof(struct simulation));

	sim->world = w;
	sim_actions_init(sim);

	return sim;
}

static enum iteration_result
assign_work(void *_sim, void *_sa)
{
	size_t j;
	uint8_t workers_needed;
	struct ent *worker;
	struct simulation *sim = _sim;
	struct sim_action *sact = _sa;
	struct action *act = &sact->act;

	if (sact->deleted) {
		return ir_cont;
	} else if (sact->cooldown) {
		--sact->cooldown;
		return ir_cont;
	}

	if (act->completion >= gcfg.actions[act->type].completed_at) {
		if (act->workers_assigned <= 0) {
			action_complete(sim, act->id);
		}

		return ir_cont;
	}

	assert(act->workers_assigned <= act->workers_requested);
	workers_needed = act->workers_requested - act->workers_assigned;

	for (j = 0; j < workers_needed; j++) {
		if (!(worker = worker_find(sim->world, sact))) {
			break;
		}

		worker_assign(worker, act);
	}

	return ir_cont;
}

static enum iteration_result
simulate_ent(void *_sim, void *_e)
{
	struct simulation *sim = _sim;
	struct ent *e = _e;
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
		if (!(random() % gcfg.misc.meander_chance)) {
			meander(sim->world->chunks, &e->pos, e->type);
			e->state |= es_modified;
		}

		goto sim_age;
	}

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

sim_age:

	if (++e->age >= gcfg.ents[e->type].lifespan) {
		if (gcfg.ents[e->type].animate) {
			over_age = ++e->age - gcfg.ents[e->type].lifespan;

			if (over_age < gcfg.misc.max_over_age
			    && (random() % (gcfg.misc.max_over_age - over_age))) {
				return ir_cont;
			}
		}

		kill_ent(sim, e);
	}

	return ir_cont;
}

static enum iteration_result
process_graveyard_iterator(void *_s, void *_id)
{
	uint16_t *id = _id;
	struct simulation *s = _s;

	world_despawn(s->world, *id);

	return ir_cont;
}

static enum iteration_result
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

	if (gcfg.ents[ne->type].animate) {
		ne->pg = calloc(1, sizeof(struct pgraph));
		pgraph_init(ne->pg, s->world->chunks);
	}

	return ir_cont;
}

void
simulate(struct simulation *sim)
{
	darr_clear_iter(sim->world->graveyard, sim, process_graveyard_iterator);
	darr_clear_iter(sim->world->spawn, sim, process_spawn_iterator);
	actions_flush(sim);

	process_environment(sim);

	hdarr_for_each(sim->actions, sim, assign_work);
	hdarr_for_each(sim->world->ents, sim, simulate_ent);

	++sim->tick;
}
