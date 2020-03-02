#include <string.h>

#include "client/sim.h"
#include "client/world_update.h"
#include "shared/messaging/server_message.h"
#include "shared/sim/alignment.h"
#include "shared/sim/ent.h"
#include "shared/types/queue.h"
#include "shared/util/log.h"
#include "shared/util/mem.h"

static void
world_copy_chunk(struct world *w, struct chunk *ck)
{
	hdarr_set(w->chunks->hd, &ck->pos, ck);
}

static void
world_apply_ent_update(struct world *w, struct sm_ent *eu)
{
	struct ent e;

	ent_init(&e);
	e.id = eu->id;
	e.pos = eu->pos;
	e.type = eu->type;
	e.alignment = eu->alignment;

	hdarr_set(w->ents, &e.id, &e);
}

static void
sim_copy_action(struct simulation *sim, struct action *act)
{
	union {
		void **vp;
		struct action **ap;

	} am = { .ap = &sim->actions.e };
	int o;

	o = get_mem(am.vp, sizeof(struct action), &sim->actions.len, &sim->actions.cap);

	memcpy(sim->actions.e + o, act, sizeof(struct action));
}

static void
sim_remove_action(struct simulation *sim, long id)
{
	size_t i;
	int j = -1;

	for (i = 0; i < sim->actions.len; i++) {
		if (sim->actions.e[i].id == id) {
			j = i;
			break;
		}
	}


	if (j == -1) {
		return;
	}

	sim->actions.len--;
	if (sim->actions.len > 0) {
		memmove(&sim->actions.e[j], &sim->actions.e[sim->actions.len], sizeof(struct action));
	}
}

static void
world_apply_update(struct simulation *sim, struct server_message *sm)
{
	switch (sm->type) {
	case server_message_ent:
		world_apply_ent_update(sim->w, ((struct sm_ent *)sm->update));

		sim->changed.ents = true;
		break;
	case server_message_chunk:
		world_copy_chunk(sim->w, &((struct sm_chunk *)sm->update)->chunk);

		sim->changed.chunks = true;
		break;
	case server_message_action:
		sim_copy_action(sim, &((struct sm_action *)sm->update)->action);
		break;
	case server_message_rem_action:
		sim_remove_action(sim, ((struct sm_rem_action *)sm->update)->id);
		break;
	}
}

bool
world_update(struct simulation *sim)
{
	struct server_message *sm;
	bool updated = false;

	while ((sm = queue_pop(sim->inbound)) != NULL) {
		updated = true;
		world_apply_update(sim, sm);
	}

	return updated;
}
