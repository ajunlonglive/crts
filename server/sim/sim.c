#define _DEFAULT_SOURCE

#include <limits.h>
#include <stdlib.h>
#include <string.h>

#include "server/sim/pathfind/meander.h"
#include "server/sim/pathfind/pathfind.h"
#include "server/sim/sim.h"
#include "server/sim/terrain.h"
#include "shared/constants/action_info.h"
#include "shared/messaging/server_message.h"
#include "shared/sim/action.h"
#include "shared/sim/alignment.h"
#include "shared/sim/ent.h"
#include "shared/sim/world.h"
#include "shared/types/geom.h"
#include "shared/types/queue.h"
#include "shared/util/log.h"

#define STEP 32

static void sim_remove_act(struct simulation *sim, int index);

struct point
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
		e->pos = p;

		alignment_adjust(e->alignment, i % 2, 9999);
	}
}

struct simulation *
sim_init(struct world *w)
{
	struct simulation *sim = malloc(sizeof(struct simulation));

	sim->world = w;
	sim->inbound = NULL;
	sim->outbound = NULL;
	sim->seq = 0;
	sim->pcap = 0;
	sim->pcnt = 0;
	sim->pending = NULL;
	sim->meander = pgraph_create(w->chunks, NULL);

	return sim;
}

static int
find_available_worker(const struct world *w, const struct action *work)
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

static int
in_range(const struct ent *e, const struct action *w)
{
	return point_in_circle(&e->pos, &w->range);
}

static void
assign_worker(struct action *act, struct ent *e)
{
	act->workers++;
	if (in_range(e, act)) {
		act->workers_in_range++;
	}
	e->task = act->id;
	e->idle = 0;
}

static void
unassign_worker(struct action *act, struct ent *e)
{
	e->task = -1;
	e->idle = 1;

	if (act != NULL) {
		act->workers--;
		act->workers_in_range--;
	}
}

static struct sim_action *
get_action(const struct simulation *sim, int id)
{
	size_t i;

	for (i = 0; i < sim->pcnt; i++) {
		if (sim->pending[i].act.id == id) {
			return &sim->pending[i];
		}
	}

	return NULL;
}

static int
get_action_id(const struct simulation *sim, int id)
{
	int i;

	for (i = 0; (size_t)i < sim->pcnt; i++) {
		if (sim->pending[i].act.id == id) {
			return i;
		}
	}

	return -1;
}

static int
get_available_tile(enum tile t, struct chunks *cnks,
	struct circle *range, struct point *p)
{
	struct point q, r;

	for (p->x = range->center.x - range->r; p->x < range->center.x + range->r; ++p->x) {
		for (p->y = range->center.y - range->r; p->y < range->center.y + range->r; ++p->y) {
			if (!point_in_circle(p, range)) {
				continue;
			}

			q = nearest_chunk(p);
			r = point_sub(p, &q);

			L("r: %d, %d", r.x, r.y);
			if (get_chunk(cnks, &q)->tiles[r.x][r.y] == t) {
				return 1;
			}
		}
	}

	return 0;
}

static enum pathfind_result
pathfind_and_update(struct simulation *sim, struct pgraph *pg, struct ent *e)
{
	enum pathfind_result r = pathfind(pg, &e->pos);
	L("pathfinding to (%d, %d), r: %d", pg->goal.x, pg->goal.y, r);
	queue_push(sim->outbound, sm_create(server_message_ent, e));
	return r;
}

static int
do_action_harvest(struct simulation *sim, struct ent *e, struct sim_action *act)
{
	struct point np = nearest_chunk(&e->pos), rp = point_sub(&e->pos, &np);
	struct chunk *chnk = get_chunk(sim->world->chunks, &np);

	enum tile *cur_tile = &chnk->tiles[rp.x][rp.y];
	int *harv = &chnk->harvested[rp.x][rp.y];

	switch (*cur_tile) {
	case tile_forest:
		(*harv)++;
		break;
	default:
		if (!points_equal(&act->local->goal, &e->pos)) {
			pathfind_and_update(sim, act->local, e);
		} else if (get_available_tile(tile_forest, sim->world->chunks,
			&act->act.range, &np)) {
			pgraph_destroy(act->local);

			act->local = pgraph_create(sim->world->chunks, &np);
		} else {
			L("failed to find available tile");
		}

		return 0;
	}

	if (*harv > 100) {
		*harv = 0;
		*cur_tile = tile_plain;
		queue_push(sim->outbound, sm_create(server_message_chunk, chnk));
		return 1;
	} else {
		return 0;
	}
}

static int
do_action(struct simulation *sim, struct ent *e, struct sim_action *act)
{
	switch (act->act.type) {
	case at_harvest:
		return do_action_harvest(sim, e, act);
	default:
		return 1;
	}
}

void
simulate(struct simulation *sim)
{
	struct ent *e;
	struct sim_action *sact;
	struct action *act;
	int is_in_range, id, j;
	size_t i;

	for (i = 0; i < sim->pcnt; i++) {
		sact = &sim->pending[i];
		act = &sact->act;

		if (sact->global == NULL) {
			queue_push(sim->outbound, sm_create(server_message_action, &sact->act));
			sact->global = pgraph_create(sim->world->chunks, &act->range.center);
			sact->local = pgraph_create(sim->world->chunks, NULL);
		}

		if (act->completion >= ACTIONS[act->type].completed_at && act->workers <= 0) {
			sim_remove_act(sim, i);
			continue;
		}

		for (j = 0; j < ACTIONS[act->type].max_workers - act->workers; j++) {
			if ((id = find_available_worker(sim->world, act)) == -1) {
				continue;
			}

			assign_worker(act, &sim->world->ents[id]);
		}
	}

	for (i = 0; i < sim->world->ecnt; i++) {
		e = &sim->world->ents[i];
		e->age++;
		if (e->satisfaction > 0) {
			e->satisfaction--;
		}

		if (e->idle) {
			if (random() % 100 > 91) {
				meander(sim->meander, &e->pos);
				queue_push(sim->outbound, sm_create(server_message_ent, e));
			}
		} else {
			if ((sact = get_action(sim, e->task)) == NULL) {
				unassign_worker(NULL, e);
				continue;
			}

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
				if (do_action(sim, e, sact)) {
					act->completion++;
				}
			} else if (!is_in_range) {
				if (pathfind_and_update(sim, sact->global, e) == pr_fail) {
					sim_remove_act(sim, get_action_id(sim, sact->act.id));
				}

				if (in_range(e, act)) {
					act->workers_in_range++;
				}
			}
		}
	}
}

struct sim_action *
sim_add_act(struct simulation *sim, const struct action *act)
{
	struct sim_action *nact;

	if (sim->pcnt + 1 >= sim->pcap) {
		sim->pcap += STEP;
		sim->pending = realloc(sim->pending, sizeof(struct sim_action) * sim->pcap);
		L("realloced pending actions buffer to size %ld", (long)sim->pcap);
	}

	nact = &sim->pending[sim->pcnt];
	sim->pcnt++;

	if (act != NULL) {
		memcpy(&nact->act, act, sizeof(struct action));
	} else {
		action_init(&nact->act);
	}

	nact->act.id = sim->seq++;
	nact->global = NULL;
	nact->local = NULL;

	return nact;
}

void
sim_remove_act(struct simulation *sim, int index)
{
	if (index < 0) {
		return;
	}

	L("removing action %ld", sim->pending[index].act.id);

	queue_push(sim->outbound, sm_create(server_message_rem_action, &sim->pending[index].act.id));

	pgraph_destroy(sim->pending[index].global);
	pgraph_destroy(sim->pending[index].local);
	memmove(&sim->pending[index], &sim->pending[sim->pcnt - 1], sizeof(struct sim_action));
	memset(&sim->pending[sim->pcnt - 1], 0, sizeof(struct sim_action));
	sim->pcnt--;
}
