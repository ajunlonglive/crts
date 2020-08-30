#include "posix.h"

#ifndef CRTS_SERVER
#define CRTS_SERVER
#endif

#include <stdbool.h>
#include <string.h>

#include "server/sim/action.h"
#include "server/sim/do_action.h"
#include "server/sim/ent_lookup.h"
#include "server/sim/pathfind/pathfind.h"
#include "server/sim/worker.h"
#include "shared/constants/globals.h"
#include "shared/util/log.h"

static bool
ent_is_applicable(struct ent *e, void *ctx)
{
	struct sim_action *sa = ctx;

	return gcfg.ents[e->type].animate
	       && !(e->state & (es_killed | es_have_task))
	       && e->alignment == sa->act.motivator;
}

static void
found_worker_cb(struct ent *e, void *ctx)
{
	struct sim_action *sa = ctx;
	worker_assign(e, &sa->act);
}

static const void *
sim_action_reverse_key(void *_sa)
{
	struct sim_action *sa = _sa;
	return &sa->act.id;
}

void
sim_actions_init(struct simulation *sim)
{
	sim->actions = hdarr_init(128, sizeof(uint8_t),
		sizeof(struct sim_action), sim_action_reverse_key);
	sim->deleted_actions = hash_init(128, 1, sizeof(uint8_t));
}

struct sim_action *
action_get(const struct simulation *sim, uint8_t id)
{
	return hdarr_get(sim->actions, &id);
}

struct sim_action *
action_add(struct simulation *sim, const struct action *act)
{
	struct sim_action nact = { 0 }, *sa;

	if (act != NULL) {
		nact.act = *act;
	} else {
		action_init(&nact.act);
	}

	nact.act.id = sim->seq++;
	pgraph_init(&nact.pg, &sim->world->chunks);
	/* TODO: dynamically determine this */
	nact.pg.trav = trav_land;

	nact.state = sas_new;


	hdarr_set(sim->actions, &nact.act.id, &nact);
	sa = hdarr_get(sim->actions, &nact.act.id);

	struct ent_lookup_ctx elctx = {
		.pg = &sa->pg,
		.usr_ctx = sa,
		.pred = ent_is_applicable,
		.cb = found_worker_cb,
	};

	ent_lookup_setup(&elctx);

	sa->elctx = elctx;

	return sa;
}

static void
action_reset(struct sim_action *sa)
{

	memset(sa->ctx, 0, sizeof(sa->ctx));

	sa->act.workers_assigned = 0;
	sa->act.workers_waiting = 0;
	sa->act.completion = 0;
	sa->cooldown = 0;
}

void
action_complete(struct simulation *sim, uint8_t id)
{
	struct sim_action *sa;

	if (!(sa = action_get(sim, id))) {
		return;
	}

	if (sa->act.flags & af_repeat) {
		action_reset(sa);
		sa->cooldown = 256;
	} else {
		action_del(sim, id);
	}
}

void
action_del(struct simulation *sim, uint8_t id)
{
	struct sim_action *sa;

	if (!(sa = action_get(sim, id)) || sa->state & sas_deleted) {
		return;
	}

	sa->state |= sas_deleted;
	hash_set(sim->deleted_actions, &sa->act.id, 1);
}

static enum iteration_result
actions_flush_iterator(void *_actions, void *_id, size_t _)
{
	struct hdarr *actions = _actions;
	uint8_t *id = _id;
	struct sim_action *sa;

	if ((sa = hdarr_get(actions, id))) {
		pgraph_destroy(&sa->pg);
		ent_lookup_teardown(&sa->elctx);
		hdarr_del(actions, id);
	}

	return ir_cont;
}

void
actions_flush(struct simulation *sim)
{
	hash_for_each_with_keys(sim->deleted_actions, sim->actions,
		actions_flush_iterator);
	hash_clear(sim->deleted_actions);
}

static void
find_workers(struct simulation *sim, struct sim_action *sa)
{
	uint32_t avail = ent_count(sim->world->ents, sa, ent_is_applicable);
	uint32_t req = estimate_work(sa, avail);

	if (!sa->elctx.init) {
		sa->elctx.origin = &sa->act.range.pos,
		/* TODO: remove 'workers_requested' from action: we don't need it anymore */
		/* sa->elctx.needed = sa->act.workers_requested, */
		sa->elctx.needed = req,
		set_action_targets(sa);

		sa->elctx.init = true;
	}

	switch (ent_lookup(sim, &sa->elctx)) {
	case rs_cont:
		return;
	case rs_done:
	case rs_fail:
		break;
	}

	L("done finding targets");

	if (!sa->elctx.found) {
		L("found no candidates");
		action_del(sim, sa->act.id);
	}

	sa->state &= ~sas_new;
	ent_lookup_reset(&sa->elctx);
}

enum iteration_result
action_process(void *_sim, void *_sa)
{
	struct simulation *sim = _sim;
	struct sim_action *sact = _sa;
	struct action *act = &sact->act;

	if (sact->state & sas_deleted) {
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

	if (sact->state & sas_new) {
		L("find ents for action %d", sact->act.id);
		find_workers(sim, sact);
	}

	if (!sact->act.workers_assigned) {
		L("deleting action with no workers");
		action_del(sim, sact->act.id);
	}

	return ir_cont;
}
