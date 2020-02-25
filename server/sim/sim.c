#ifndef CRTS_SERVER
#define CRTS_SERVER
#endif

#define _XOPEN_SOURCE 500

#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>

#include "server/sim/action.h"
#include "server/sim/do_action.h"
#include "server/sim/pathfind/meander.h"
#include "server/sim/pathfind/pathfind.h"
#include "server/sim/sim.h"
#include "server/sim/terrain.h"
#include "server/sim/worker.h"
#include "shared/constants/globals.h"
#include "shared/messaging/server_message.h"
#include "shared/sim/alignment.h"
#include "shared/sim/ent.h"

static struct point
get_valid_spawn(struct chunks *chunks)
{
	struct point p = { 0, 0 }, q;
	const struct chunk *ck;
	int i, j;

	while (1) {
		p.x += CHUNK_SIZE;
		ck = get_chunk(chunks, &p);

		for (i = 0; i < CHUNK_SIZE; i++) {
			for (j = 0; j < CHUNK_SIZE; j++) {
				if (ck->tiles[i][j] < tile_forest) {
					q.x = i; q.y = j;
					return point_add(&p, &q);
				}
			}
		}
	}
}

void
populate(struct simulation *sim)
{
	size_t i;
	struct ent *e;
	struct point p = get_valid_spawn(sim->world->chunks);

	for (i = 0; i < 100; i++) {
		e = world_spawn(sim->world);
		e->type = et_worker;
		e->pos = p;

		alignment_adjust(e->alignment, i % 2, 9999);
	}
}

struct simulation *
sim_init(struct world *w)
{
	struct simulation *sim = calloc(1, sizeof(struct simulation));

	sim->world = w;

	return sim;
}

enum pathfind_result
pathfind_and_update(struct simulation *sim, struct pgraph *pg, struct ent *e)
{
	enum pathfind_result r = pathfind(pg, &e->pos);
	queue_push(sim->outbound, sm_create(server_message_ent, e));
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
			queue_push(sim->outbound,
				sm_create(server_message_action, &sact->act));
			sact->global = pgraph_create(sim->world->chunks,
				&act->range.center);
		}

		sact->global->cooldown = false;

		if (act->completion >= gcfg.actions[act->type].completed_at
		    && act->workers_assigned <= 0) {
			action_del(sim, act->id);
			continue;
		}

		assert(act->workers_assigned <= act->workers_requested);
		workers_needed = act->workers_requested - act->workers_assigned;

		for (j = 0; j < workers_needed; j++) {
			if ((worker = worker_find(sim->world, act)) == NULL) {
				continue;
			}

			worker_assign(worker, act);
		}
	}

}

void
simulate(struct simulation *sim)
{
	struct ent *e;
	struct sim_action *sact;
	struct action *act;
	int is_in_range;
	size_t i;

	assign_work(sim);

	for (i = 0; i < sim->world->ents.len; i++) {
		e = &sim->world->ents.e[i];

		if (!gcfg.ents[e->type].animate) {
			continue;
		}

		if (e->satisfaction > 0) {
			e->satisfaction--;
		}

		if (e->idle && random() % 100 > 91) {
			meander(sim->world->chunks, &e->pos);
			queue_push(sim->outbound, sm_create(server_message_ent, e));
		} else {
			if ((sact = action_get(sim, e->task)) == NULL) {
				worker_unassign(e, NULL);
				continue;
			}

			act = &sact->act;
			is_in_range = point_in_circle(&e->pos, &act->range);

			if (act->completion >= gcfg.actions[act->type].completed_at) {
				e->satisfaction += gcfg.actions[act->type].satisfaction;
				alignment_adjust(
					e->alignment,
					act->motivator,
					gcfg.actions[act->type].satisfaction
					);

				worker_unassign(e, act);
			} else if (!is_in_range) {
				if (pathfind_and_update(sim, sact->global, e) == pr_fail) {
					action_del(sim, sact->act.id);
				}

				if (point_in_circle(&e->pos, &act->range)) {
					act->workers_in_range++;
				}
			} else if (is_in_range && act->workers_in_range >= act->workers_requested) {
				switch (do_action(sim, e, sact)) {
				case ar_done:
					act->completion++;
					break;
				case ar_fail:
					action_del(sim, sact->act.id);
					break;
				case ar_cont:
					break;
				}
			}
		}
	}
}
