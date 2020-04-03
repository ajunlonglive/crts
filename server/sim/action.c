#ifndef CRTS_SERVER
#define CRTS_SERVER
#endif

#define _XOPEN_SOURCE 500

#include <stdbool.h>
#include <string.h>

#include "server/sim/action.h"
#include "server/sim/do_action.h"
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
	/* TODO: dynamically determine this */
	nact.pg.trav = trav_land;

	nact.state = sas_new;

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
		hash_destroy(sa->ent_blacklist);
		pgraph_destroy(&sa->pg);
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

static bool
ent_is_applicable(struct ent *e, uint8_t mot)
{
	return gcfg.ents[e->type].animate
	       && !(e->state & (es_killed | es_have_task))
	       && e->alignment == mot;

	//&& !action_ent_blacklisted(ctx->sa, e))
}

struct ascb_ctx {
	struct simulation *sim;
	struct sim_action *sa;
	uint16_t found;
	uint16_t needed;
	uint32_t checked;
	uint32_t total;
};

static enum iteration_result
check_workers_at(void *_ctx, struct ent *e)
{
	struct ascb_ctx *ctx = _ctx;


	if (ent_is_applicable(e, ctx->sa->act.motivator)) {
		worker_assign(e, &ctx->sa->act);

		if (++ctx->found >= ctx->needed) {
			return ir_done;
		}

		++ctx->checked;
	}

	return ir_cont;
}

static enum result
ascb(void *_ctx, const struct point *p)
{
	struct ascb_ctx *ctx = _ctx;

	for_each_ent_at(&ctx->sim->eb, ctx->sim->world->ents, p,
		ctx, check_workers_at);

	if (ctx->found >= ctx->needed || ctx->checked >= ctx->total) {
		return rs_done;
	} else {
		return rs_cont;
	}
}

static void
find_workers(struct simulation *sim, struct sim_action *sa)
{
	set_action_targets(sa);

	struct ascb_ctx ctx = {
		sim, sa,
		0, sa->act.workers_requested - sa->act.workers_assigned,
		0, hdarr_len(sim->world->ents)
	};

	astar(&sa->pg, NULL, &ctx, ascb);

	if (ctx.found == 0) {
		L("found no candidates");
		action_del(sim, sa->act.id);
	}
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
		find_workers(sim, sact);
		sact->state &= ~sas_new;
	}

	return ir_cont;
}

