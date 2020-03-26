#ifndef CRTS_SERVER
#define CRTS_SERVER
#endif

#define _XOPEN_SOURCE 500

#include <stdbool.h>
#include <string.h>

#include "server/sim/action.h"
#include "server/sim/pathfind/pgraph.h"
#include "server/sim/sim.h"
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
		memcpy(&nact.act, act, sizeof(struct action));
	} else {
		action_init(&nact.act);
	}

	nact.act.id = sim->seq++;
	nact.ent_blacklist = hash_init(128, 1, sizeof(uint32_t));

	hdarr_set(sim->actions, &nact.act.id, &nact);

	return hdarr_get(sim->actions, &nact.act.id);
}

void
action_del(struct simulation *sim, uint8_t id)
{
	struct sim_action *sa;

	if (!(sa = action_get(sim, id)) || sa->deleted) {
		return;
	}

	pgraph_destroy(sa->global);
	pgraph_destroy(sa->local);
	hash_destroy(sa->ent_blacklist);
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
