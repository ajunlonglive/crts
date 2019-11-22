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

static int sim_rand_coin(struct simulation *sim, int coins)
{
	int r;

	random_r(&sim->prng, &r);

	return r < (RAND_MAX / coins + 1);
}

void populate(struct simulation *sim)
{
	size_t i;
	struct ent *e;

	for (i = 0; i < 100; i++) {
		e = world_spawn(sim->world);
		random_r(&sim->prng, &e->pos.x);
		random_r(&sim->prng, &e->pos.y);
		e->pos.x %= 10;
		e->pos.y %= 10;
	}
}

struct simulation *sim_init(struct world *w, int seed)
{
	struct simulation *sim = malloc(sizeof(struct simulation));

	sim->world = w;
	sim->inbound = NULL;
	sim->outbound = NULL;
	sim->seq = 0;
	sim->pcap = 0;
	sim->pcnt = 0;
	sim->pending = NULL;

	sim->statebuf = malloc(STATEBUF_LEN);
	sim->prng.state = NULL;
	initstate_r(seed, sim->statebuf, STATEBUF_LEN, &sim->prng);

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

void simulate(struct simulation *sim)
{
	struct ent *e;
	struct action *act;
	size_t i;
	int id;

	L("simulating world with %d ents, %d tasks", sim->world->ecnt, sim->pcnt);

	for (i = 0; i < sim->pcnt; i++) {
		act = &sim->pending[i];
		//L("task %3d | %3d | %3d/%3d", i, act->workers, act->completion, ACTIONS[act->type].completed_at);

		if (act->completion >= ACTIONS[act->type].completed_at && act->workers <= 0) {
			L("task %3d | %3d | %3d/%3d", i, act->workers, act->completion, ACTIONS[act->type].completed_at);
			sim_remove_act(sim, i);
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
			act = &sim->pending[e->task];

			if (act->completion >= ACTIONS[act->type].completed_at) {
				L("ent %d stopping action %d", i, e->task);
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

				queue_push(sim->outbound, ent_update_init(e));
			}
		} else if (e->age > 20 && e->age < 80 && sim_rand_coin(sim, 1000)) {
			act = sim_add_act(sim, NULL);
			act->type = action_type_0;
			act->motivator = 0;
			act->range.center = e->pos;

			assign_worker(act, e);
		} else if (e->age > 100 && sim_rand_coin(sim, 100)) {
			world_despawn(sim->world, i);
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

	sim->pcnt++;
	nact = &sim->pending[sim->pcnt - 1];
	nact->id = sim->seq++;

	if (act != NULL)
		nact = memcpy(nact, act, sizeof(struct action));

	return nact;
}

void sim_remove_act(struct simulation *sim, int index)
{
	memcpy(&sim->pending[index], &sim->pending[sim->pcnt - 1], sizeof(struct action));
	memset(&sim->pending[sim->pcnt - 1], 0, sizeof(struct action));
	sim->pcnt--;
}
