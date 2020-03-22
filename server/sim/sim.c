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
#include "shared/sim/alignment.h"
#include "shared/sim/ent.h"
#include "shared/types/result.h"
#include "shared/util/log.h"

static struct point
get_valid_spawn(struct chunks *chunks)
{
	struct point p = { random() % 100, random() % 100 }, q;
	const struct chunk *ck;
	int i, j;

	while (1) {
		p.x += CHUNK_SIZE;
		ck = get_chunk(chunks, &p);

		for (i = 0; i < CHUNK_SIZE; i++) {
			for (j = 0; j < CHUNK_SIZE; j++) {
				if (tile_is_traversable(ck->tiles[i][j])) {
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
	struct point p = get_valid_spawn(sim->world->chunks);

	for (i = 0; i < amnt; i++) {
		e = world_spawn(sim->world);
		e->type = et_worker;
		e->pos = p;

		alignment_adjust(e->alignment, algn, 9999);
	}
}

void
kill_ent(struct simulation *sim, struct ent *e)
{
	if (!(e->state & es_killed)) {
		darr_push(sim->world->graveyard, &e->id);
		e->state |= (es_killed | es_modified);
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

	populate(sim, 255, nm);

	return nm;
}

struct simulation *
sim_init(struct world *w)
{
	struct simulation *sim = calloc(1, sizeof(struct simulation));

	sim->world = w;

	return sim;
}

enum result
pathfind_and_update(struct simulation *sim, struct pgraph *pg, struct ent *e)
{
	enum result r = pathfind(pg, &e->pos);
	e->state |= es_modified;
	return r;
}

static void
assign_work(struct simulation *sim)
{
	size_t i, j;
	uint8_t workers_needed;
	struct sim_action *sact;
	struct action *act;
	struct ent *worker;

	for (i = 0; i < sim->actions.len; i++) {
		sact = &sim->actions.e[i];
		act = &sact->act;

		if (sact->global == NULL) {
			sact->global = pgraph_create(sim->world->chunks,
				&act->range.center);
		}

		if (act->completion >= gcfg.actions[act->type].completed_at) {
			if (act->workers_assigned <= 0) {
				action_del(sim, act->id);
			}

			continue;
		}

		assert(act->workers_assigned <= act->workers_requested);
		workers_needed = act->workers_requested - act->workers_assigned;

		for (j = 0; j < workers_needed; j++) {
			if ((worker = worker_find(sim->world, sact)) == NULL) {
				continue;
			}

			worker_assign(worker, act);
		}
	}
}

static enum iteration_result
simulate_ent(void *_sim, void *_e)
{
	struct simulation *sim = _sim;
	struct ent *e = _e;
	struct sim_action *sact;

	if (e->state & es_killed) {
		return ir_cont;
	} else if (!gcfg.ents[e->type].animate) {
		goto sim_age;
	}

	if (e->satisfaction > 0) {
		e->satisfaction--;
	}

	if (!(e->state & es_have_task)) {
		if (random() % 10000 > 9900) {
			meander(sim->world->chunks, &e->pos);
			e->state |= es_modified;
		}

		goto sim_age;
	}

	if ((sact = action_get(sim, e->task)) == NULL) {
		worker_unassign(e, NULL);
	} else if (sact->act.completion >= gcfg.actions[sact->act.type].completed_at) {
		/*
		   e->satisfaction += gcfg.actions[sact->act.type].satisfaction;

		   alignment_adjust(
		        e->alignment,
		        sact->act.motivator,
		        gcfg.actions[sact->act.type].satisfaction
		        );
		 */

		worker_unassign(e, &sact->act);
	} else {
		switch (do_action(sim, e, sact)) {
		case rs_done:
			sact->act.completion++;
			break;
		case rs_fail:
			L("action %d failed", sact->act.id);
			action_del(sim, sact->act.id);
			break;
		case rs_cont:
			break;
		}
	}

sim_age:

	if (++e->age >= gcfg.ents[e->type].lifespan) {
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
	ne->state = es_modified;
	/* TODO: memory leak since reference to alignment is lost */
	ne->alignment = e->alignment;

	return ir_cont;
}

void
simulate(struct simulation *sim)
{
	darr_clear_iter(sim->world->graveyard, sim, process_graveyard_iterator);
	darr_clear_iter(sim->world->spawn, sim, process_spawn_iterator);

	process_environment(sim);

	assign_work(sim);

	hdarr_for_each(sim->world->ents, sim, simulate_ent);

	++sim->tick;
}
