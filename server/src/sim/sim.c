#define _DEFAULT_SOURCE

#include <limits.h>
#include <stdlib.h>
#include <string.h>

#include "constants/action_info.h"
#include "messaging/server_message.h"
#include "pathfind/pathfind.h"
#include "sim.h"
#include "sim/action.h"
#include "sim/alignment.h"
#include "sim/ent.h"
#include "sim/world.h"
#include "terrain.h"
#include "types/geom.h"
#include "types/queue.h"
#include "util/log.h"

#define STEP 32

static void sim_remove_act(struct simulation *sim, int index);

struct point get_valid_spawn(struct hash *chunks)
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

void populate(struct simulation *sim)
{
	size_t i;
	struct ent *e;
	struct point p = get_valid_spawn(sim->world->chunks);

	for (i = 0; i < 100; i++) {
		e = world_spawn(sim->world);
		e->pos = p;

		alignment_adjust(e->alignment, i % 2, 9999);
	}
}

struct simulation *sim_init(struct world *w)
{
	struct simulation *sim = malloc(sizeof(struct simulation));

	sim->world = w;
	sim->inbound = NULL;
	sim->outbound = NULL;
	sim->seq = 0;
	sim->pcap = 0;
	sim->pcnt = 0;
	sim->pending = NULL;

	return sim;
}

static int find_available_worker(const struct world *w, const struct action *work)
{
	size_t i, ci = -1;
	struct ent *e;
	int closest_dist = INT_MAX, dist;

	for (i = 0; i < w->ecnt; i++) {
		e = &w->ents[i];

		if (e->idle && e->alignment->max == work->motivator) {

			dist = distance_point_to_circle(&e->pos, &work->range);

			if (dist < closest_dist) {
				closest_dist = dist;
				ci = i;
			}
		}
	}

	return ci;
}

static int in_range(const struct ent *e, const struct action *w)
{
	return point_in_circle(&e->pos, &w->range);
}

static void assign_worker(struct action *act, struct ent *e)
{
	act->workers++;
	if (in_range(e, act))
		act->workers_in_range++;
	e->task = act->id;
	e->idle = 0;
}

static void unassign_worker(struct action *act, struct ent *e)
{
	e->task = -1;
	e->idle = 1;
	act->workers--;
	act->workers_in_range--;
}

static struct sim_action *get_action(const struct simulation *sim, int id)
{
	size_t i;

	for (i = 0; i < sim->pcnt; i++)
		if (sim->pending[i].act.id == id)
			return &sim->pending[i];

	return NULL;
}

void simulate(struct simulation *sim)
{
	struct ent *e;
	struct sim_action *sact;
	struct action *act;
	int is_in_range, id, j;
	size_t i;

	for (i = 0; i < sim->pcnt; i++) {
		sact = &sim->pending[i];
		act = &sact->act;

		if (sact->g == NULL) {
			queue_push(sim->outbound, sm_create(server_message_action, &sact->act));
			sact->g = tile_pg_create(sim->world->chunks, &act->range.center);
		}

		if (act->completion >= ACTIONS[act->type].completed_at && act->workers <= 0) {
			sim_remove_act(sim, i);
			continue;
		}

		for (j = 0; j < ACTIONS[act->type].max_workers - act->workers; j++) {
			if ((id = find_available_worker(sim->world, act)) == -1)
				continue;

			assign_worker(act, &sim->world->ents[id]);
		}
	}

	for (i = 0; i < sim->world->ecnt; i++) {
		e = &sim->world->ents[i];
		e->age++;
		if (e->satisfaction > 0)
			e->satisfaction--;

		if (e->idle) {
			/*
			   if (random() % 100 > 91) {
			        meander(sim->world->chunks, &e->pos);
			        queue_push(sim->outbound, sm_create(server_message_ent, e));
			   }
			 */

		} else {
			sact = get_action(sim, e->task);
			act = &sact->act;
			is_in_range = in_range(e, act);

			if (act->completion >= ACTIONS[act->type].completed_at) {
				e->satisfaction += ACTIONS[act->type].satisfaction;
				alignment_adjust(
					e->alignment,
					act->motivator,
					ACTIONS[act->type].satisfaction
					);

				unassign_worker(act, e);
			} else if (is_in_range && act->workers_in_range >= ACTIONS[act->type].min_workers) {
				act->completion++;
			} else if (!is_in_range) {
				pathfind(sact->g, &e->pos);

				queue_push(sim->outbound, sm_create(server_message_ent, e));

				if (in_range(e, act))
					act->workers_in_range++;
			}
		}
	}
}

struct sim_action *sim_add_act(struct simulation *sim, const struct action *act)
{
	struct sim_action *nact;

	if (sim->pcnt + 1 >= sim->pcap) {
		sim->pcap += STEP;
		sim->pending = realloc(sim->pending, sizeof(struct sim_action) * sim->pcap);
		L("realloced pending actions buffer to size %ld", (long)sim->pcap);
	}

	nact = &sim->pending[sim->pcnt];
	sim->pcnt++;

	if (act != NULL)
		memcpy(&nact->act, act, sizeof(struct action));
	else
		action_init(&nact->act);

	nact->act.id = sim->seq++;
	nact->g = NULL;

	return nact;
}

void sim_remove_act(struct simulation *sim, int index)
{
	if (index < 0)
		return;

	L("removing action %ld", sim->pending[index].act.id);

	queue_push(sim->outbound, sm_create(server_message_rem_action, &sim->pending[index].act.id));

	memmove(&sim->pending[index], &sim->pending[sim->pcnt - 1], sizeof(struct sim_action));
	memset(&sim->pending[sim->pcnt - 1], 0, sizeof(struct sim_action));
	sim->pcnt--;
}
