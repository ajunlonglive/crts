#ifndef CRTS_SERVER
#define CRTS_SERVER
#endif

#define _XOPEN_SOURCE 500

#include <stdbool.h>
#include <string.h>

#include "server/sim/action.h"
#include "server/sim/ent_buckets.h"
#include "server/sim/pathfind/pathfind.h"
#include "server/sim/sim.h"
#include "server/sim/terrain.h"
#include "server/sim/worker.h"
#include "shared/constants/globals.h"
#include "shared/sim/ent.h"
#include "shared/types/hash.h"
#include "shared/util/log.h"
#include "shared/util/mem.h"

static void *
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
	struct sim_action nact = { 0 };

	if (act != NULL) {
		nact.act = *act;
	} else {
		action_init(&nact.act);
	}

	nact.act.id = sim->seq++;
	nact.ent_blacklist = hash_init(128, 1, sizeof(uint32_t));
	pgraph_init(&nact.pg, sim->world->chunks);

	hdarr_set(sim->actions, &nact.act.id, &nact);

	return hdarr_get(sim->actions, &nact.act.id);
}

static void
action_reset(struct sim_action *sa)
{

	memset(sa->ctx, 0, sizeof(sa->ctx));

	sa->act.workers_assigned = 0;
	sa->act.workers_waiting = 0;
	sa->act.completion = 0;

	hash_clear(sa->ent_blacklist);

	if (sa->hash) {
		hash_clear(sa->hash);
	}

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

	if (!(sa = action_get(sim, id)) || sa->deleted) {
		return;
	}

	hash_destroy(sa->ent_blacklist);
	pgraph_destroy(&sa->pg);
	sa->deleted = true;
	hash_set(sim->deleted_actions, &sa->act.id, 1);
}

static enum iteration_result
actions_flush_iterator(void *_actions, void *_id, size_t _)
{
	struct hdarr *actions = _actions;
	uint8_t *id = _id;

	hdarr_del(actions, id);

	return ir_cont;
}

void
actions_flush(struct simulation *sim)
{
	hash_for_each_with_keys(sim->deleted_actions, sim->actions,
		actions_flush_iterator);
	hash_clear(sim->deleted_actions);
}

void
action_ent_blacklist(struct sim_action *sa, const struct ent *e)
{
	hash_set(sa->ent_blacklist, &e->id, 1);
}

bool
action_ent_blacklisted(const struct sim_action *sa, const struct ent *e)
{
	const size_t *sp;

	return (sp = hash_get(sa->ent_blacklist, &e->id)) != NULL && *sp == 1;
}

struct ascb_ctx {
	struct simulation *sim;
	struct sim_action *sa;
	uint16_t found;
	uint16_t needed;
	const struct point *p;
};

static enum iteration_result
check_workers_at(void *_ctx, struct ent *e)
{
	struct ascb_ctx *ctx = _ctx;

	L("checking ent: %d, (%d %d %d)", e->id,
		gcfg.ents[e->type].animate,
		!(e->state & (es_killed | es_have_task)),
		e->alignment == ctx->sa->act.motivator
		);

	if (gcfg.ents[e->type].animate
	    && !(e->state & (es_killed | es_have_task))
	    && e->alignment == ctx->sa->act.motivator
	    //&& !action_ent_blacklisted(ctx->sa, e)) {
	    ) {
		L("adding worker");
		worker_assign(e, &ctx->sa->act);

		if (++ctx->found >= ctx->needed) {
			return ir_done;
		}
	}

	return ir_cont;
}

static enum result
ascb(void *_ctx, const struct point *p)
{
	struct ascb_ctx *ctx = _ctx;

	ctx->p = p;

	for_each_ent_at(&ctx->sim->eb, ctx->sim->world->ents, p,
		ctx, check_workers_at);

	return ctx->found >= ctx->needed ? rs_done : rs_cont;
}

static void
find_workers2(struct simulation *sim, struct sim_action *sa)
{
	if (sa->act.workers_assigned >= sa->act.workers_requested) {
		return;
	}
	L("finding workers for action %d, (%s)", sa->act.id, gcfg.actions[sa->act.type].name);

	struct ascb_ctx ctx = {
		sim, sa, 0,
		sa->act.workers_requested - sa->act.workers_assigned,
	};

	pgraph_set(&sa->pg, &sa->act.range.center, trav_land);

	astar(&sa->pg, NULL, &ctx, ascb);

	if (ctx.found == 0) {
		L("found no candidates");
	}
}

enum iteration_result
action_process(void *_sim, void *_sa)
{
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

	find_workers2(sim, sact);

	return ir_cont;
}

