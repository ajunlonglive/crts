#define _DEFAULT_SOURCE

#include <stdlib.h>
#include <string.h>
#include "update.h"
#include "geom.h"
#include "action.h"
#include "globals.h"
#include "world.h"
#include "queue.h"
#include "sim.h"
#include "log.h"

#define STEP 32

static void sim_remove_act(struct simulation *sim, int index);

void populate(struct simulation *sim)
{
	size_t i;
	struct ent *e;

	for (i = 0; i < 100; i++) {
		e = world_spawn(sim->world);
		e->pos.x = (random() % 90) + 1;
		e->pos.y = (random() % 30) + 1;
	}
}

static void add_random_action(struct simulation *sim)
{
	struct action *act = sim_add_act(sim, NULL);

	act->type = action_type_1;
	act->motivator = 0;
	struct point p = { .x = random() % 110, .y = random() % 20 };
	act->range.center = p;
	act->range.r = 2 + (random() % 10);
	L("added action %d @ %d, %d, r: %d", act->id, p.x, p.y, act->range.r);
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

	add_random_action(sim);
	add_random_action(sim);
	add_random_action(sim);
	add_random_action(sim);
	add_random_action(sim);

	return sim;
}

static int find_free_worker(const struct world *w, const struct action *work)
{
	size_t i;
	struct ent *e;

	for (i = 0; i < w->ecnt; i++) {
		e = &w->ents[i];

		if (e->idle && e->alignment->max == work->motivator)
			return i;
	}


	return -1;
}

static int in_range(const struct ent *e, const struct action *w)
{
	return point_in_circle(&e->pos, &w->range);
}

static void assign_worker(struct action *act, struct ent *e)
{
	act->workers++;
	e->task = act->id;
	e->idle = 0;
}

static struct action *get_action(const struct simulation *sim, int id)
{
	size_t i;

	for (i = 0; i < sim->pcnt; i++)
		if (sim->pending[i].id == id)
			return &sim->pending[i];

	return NULL;
}

void simulate(struct simulation *sim)
{
	struct ent *e;
	struct action *act;
	size_t i;
	int id;

	for (i = 0; i < sim->pcnt; i++) {
		act = &sim->pending[i];

		if (act->completion >= ACTIONS[act->type].completed_at && act->workers <= 0) {
			sim_remove_act(sim, i);
			add_random_action(sim);
			continue;
		}

		if (act->workers >= ACTIONS[act->type].max_workers)
			continue;

		if ((id = find_free_worker(sim->world, act)) == -1)
			continue;

		assign_worker(act, &sim->world->ents[id]);
	}

	for (i = 0; i < sim->world->ecnt; i++) {
		e = &sim->world->ents[i];
		e->age++;
		if (e->satisfaction > 0)
			e->satisfaction--;

		if (!e->idle) {
			act = get_action(sim, e->task);

			if (act->completion >= ACTIONS[act->type].completed_at) {
				e->task = -1;
				e->idle = 1;
				e->satisfaction += ACTIONS[act->type].satisfaction;
				alignment_adjust(
					e->alignment,
					act->motivator,
					ACTIONS[act->type].satisfaction
					);

				act->workers--;
			} else if (in_range(e, act)) {
				act->completion++;
			} else {
				pathfind(&e->pos, &act->range.center);

				/*
				   if (e->pos.x == 0 && e->pos.y == 0) {
				        L("pathfind got us to 0, 0");
				        L("while heading to the following action");
				        action_inspect(act);
				        tpos = e->pos;
				        pathfind(&tpos, &act->range.center);
				        L("a further pathfind resuls in: %d %d", tpos.x, tpos.y);
				   }
				 */


				queue_push(sim->outbound, ent_update_init(e));
			}
		}
	}
}

struct action *sim_add_act(struct simulation *sim, const struct action *act)
{
	struct action *nact;

	if (sim->pcnt + 1 >= sim->pcap) {
		sim->pcap += STEP;
		sim->pending = realloc(sim->pending, sizeof(struct action) * sim->pcap);
		L("realloced pending actions buffer to size %d", sim->pcap);
	}

	nact = &sim->pending[sim->pcnt];
	sim->pcnt++;
	nact->id = sim->seq++;

	if (act != NULL)
		nact = memcpy(nact, act, sizeof(struct action));

	return nact;
}

void sim_remove_act(struct simulation *sim, int index)
{
	L("removing action %d", sim->pending[index].id);
	memcpy(&sim->pending[index], &sim->pending[sim->pcnt - 1], sizeof(struct action));
	memset(&sim->pending[sim->pcnt - 1], 0, sizeof(struct action));
	sim->pcnt--;
}
