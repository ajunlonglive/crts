#define _DEFAULT_SOURCE

#include <limits.h>
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
		e->pos.x = (random() % 20) - 10;
		e->pos.y = (random() % 20) - 10;

		alignment_adjust(e->alignment, i % 2, 9999);
	}
}

/*
   static void add_random_action(struct simulation *sim)
   {
        struct action *act = sim_add_act(sim, NULL);

        act->type = action_type_1;
        act->motivator = 0;
        struct point p = { .x = random() % 110, .y = random() % 20 };
        act->range.center = p;
        act->range.r = 5;// + (random() % 10);
        L("added action %d @ %d, %d, r: %d", act->id, p.x, p.y, act->range.r);
   }
 */

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

/*
   static struct action *find_available_job(const struct simulation *sim, struct ent *e)
   {
        size_t i;
        struct action *act, *closest;

        int dist, closest_dist = INT_MAX;

        for (i = 0; i < sim->pcnt; i++) {
                act = &sim->pending[i];

                if (e->alignment->max == act->motivator
                    && act->workers < ACTIONS[act->type].max_workers
                    && act->completion < ACTIONS[act->type].completed_at) {
                        dist = distance_point_to_circle(&e->pos, &act->range);

                        if (dist < closest_dist || closest_dist < 0)
                                closest = act;
                }
        }

        return NULL;
   }
 */

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
	int is_in_range, id, j;
	size_t i;

	for (i = 0; i < sim->pcnt; i++) {
		act = &sim->pending[i];

		if (act->completion >= ACTIONS[act->type].completed_at && act->workers <= 0) {
			sim_remove_act(sim, i);
			continue;
		}

		//action_inspect(act);
		//add_random_action(sim);

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
			if (random() % 100 > 91) {
				switch (random() % 4) {
				case 0:
					e->pos.x++;
					break;
				case 1:
					e->pos.x--;
					break;
				case 2:
					e->pos.y++;
					break;
				case 3:
					e->pos.y--;
					break;
				}

				queue_push(sim->outbound, ent_update_init(e));
			}

		} else {
			act = get_action(sim, e->task);
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
				pathfind(&e->pos, &act->range.center);

				queue_push(sim->outbound, ent_update_init(e));

				if (in_range(e, act))
					act->workers_in_range++;
			}
		}
		//L("ent %d, job: %d(%p) %c, pos: %4d, %4d", i, e->task, act, e->idle ? 'i' : 'w', e->pos.x, e->pos.y);
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

	if (act != NULL)
		nact = memcpy(nact, act, sizeof(struct action));
	else
		action_init(nact);

	nact->id = sim->seq++;

	return nact;
}

void sim_remove_act(struct simulation *sim, int index)
{
	L("removing action %d", sim->pending[index].id);
	memmove(&sim->pending[index], &sim->pending[sim->pcnt - 1], sizeof(struct action));
	memset(&sim->pending[sim->pcnt - 1], 0, sizeof(struct action));
	sim->pcnt--;
}
