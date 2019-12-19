#include <string.h>

#include "messaging/server_message.h"
#include "sim.h"
#include "sim/alignment.h"
#include "sim/ent.h"
#include "types/queue.h"
#include "util/log.h"
#include "util/mem.h"
#include "world_update.h"

static struct ent *find_or_create_ent(struct world *w, int id)
{
	size_t i;

	if (w->ecnt == 0 || (size_t)id > w->ecnt - 1) {
		if (w->ecap == 0 || (size_t)id > w->ecap - 1) {
			w->ecap = id + (ENT_STEP - id % ENT_STEP);
			L("reallocating ent pool to %ld ", (long)w->ecap);
			w->ents = realloc(w->ents, sizeof(struct ent) * w->ecap);
		}

		for (i = w->ecnt; i <= (size_t)id; i++) {
			ent_init(&w->ents[i]);
			w->ents[i].id = i;
		}

		w->ecnt = id + 1;
	}

	return &w->ents[id];
}

static void world_copy_chunk(struct world *w, struct chunk *ck)
{
	if (hash_get(w->chunks, &ck->pos) != NULL)
		return;

	L("applying chunk update");

	struct chunk *mck = NULL;

	chunk_init(&mck);
	memcpy(mck, ck, sizeof(struct chunk));
	L("setting chunk @ %d, %d", mck->pos.x, mck->pos.y);

	hash_set(w->chunks, &mck->pos, mck);
}

static void world_apply_ent_update(struct world * w, struct sm_ent *eu)
{
	struct ent *e;

	e = find_or_create_ent(w, eu->id);
	e->pos = eu->pos;
	e->alignment->max = eu->alignment;
}

static void sim_copy_action(struct simulation *sim, struct action *act)
{
	union {
		void **vp;
		struct action **ap;

	} am = { .ap = &sim->actions.e };
	int o;

	o = get_mem(am.vp, sizeof(struct action), &sim->actions.len, &sim->actions.cap);

	memcpy(sim->actions.e + o, act, sizeof(struct action));
}

static void sim_remove_action(struct simulation *sim, long id)
{
	size_t i;
	int j = -1;

	for (i = 0; i < sim->actions.len; i++) {
		if (sim->actions.e[i].id == id) {
			j = i;
			break;
		}
	}


	if (j == -1)
		return;

	sim->actions.len--;
	if (sim->actions.len > 0)
		memmove(&sim->actions.e[j], &sim->actions.e[sim->actions.len], sizeof(struct action));
}

static void world_apply_update(struct simulation *sim, struct server_message *sm)
{
	switch (sm->type) {
	case server_message_ent:
		world_apply_ent_update(sim->w, ((struct sm_ent *)sm->update));
		break;
	case server_message_chunk:
		world_copy_chunk(sim->w, &((struct sm_chunk *)sm->update)->chunk);
		break;
	case server_message_action:
		sim_copy_action(sim, &((struct sm_action *)sm->update)->action);
		break;
	case server_message_rem_action:
		sim_remove_action(sim, ((struct sm_rem_action *)sm->update)->id);
		break;
	}
}

void *world_update(struct simulation *sim)
{
	struct server_message *sm;

	while (1) {
		sm = queue_pop(sim->inbound, 1);
		world_apply_update(sim, sm);
	}
}
