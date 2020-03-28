#include <string.h>

#include "client/sim.h"
#include "client/hiface.h"
#include "client/world_update.h"
#include "shared/messaging/server_message.h"
#include "shared/net/net_ctx.h"
#include "shared/sim/ent.h"
#include "shared/types/darr.h"
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
	struct ent *e, re = { 0 };
	size_t i;

	for (i = 0; i < SM_ENT_LEN; ++i) {
		if ((eu->updates[i].type & 0xffff) == eut_none) {
			break;
		} else if ((e = hdarr_get(w->ents, &eu->updates[i].id)) == NULL) {
			hdarr_set(w->ents, &eu->updates[i].id, &re);
			e = hdarr_get(w->ents, &eu->updates[i].id);
			e->id = eu->updates[i].id;
			e->alignment = (eu->updates[i].type >> 16) & 0x00ff;
			e->type = eu->updates[i].type >> 24;
		}


		switch (eu->updates[i].type & 0xffff) {
		case eut_kill:
			world_despawn(w, e->id);
			break;
		case eut_pos:
			e->pos = eu->updates[i].ud.pos;
			break;
		case eut_align:
			e->alignment = eu->updates[i].ud.alignment;
			break;
		}
	}
}

static void
sim_remove_action(struct simulation *sim, uint8_t id)
{
	L("removing action %d", id);

	/* TODO: remove assert when this is no longer guaranteed by id type */
	//assert(id < ACTION_HISTORY_SIZE);

	size_t i;

	for (i = 0; i < sim->action_history_len; ++i) {
		if (sim->action_history_order[i] == id) {
			if (i + 1 < sim->action_history_len) {
				memmove(&sim->action_history_order[i],
					&sim->action_history_order[i + 1],
					sim->action_history_len - i
					);
			}

			--sim->action_history_len;
			break;
		}
	}

	sim->action_history[id].type = at_none;

	sim->changed.actions = true;
}

static void
apply_world_info(struct simulation *sim, struct sm_world_info *wi)
{
	sim->server_world.ents = wi->ents;
}

static enum iteration_result
world_apply_update(void *_sim, void *_sm)
{
	struct simulation *sim = _sim;
	struct server_message *sm = _sm;

	switch (sm->type) {
	case server_message_ent:
		world_apply_ent_update(sim->w, &sm->msg.ent);

		sim->changed.ents = true;
		break;
	case server_message_chunk:
		L("got chunk update %d, %d", sm->msg.chunk.chunk.pos.x, sm->msg.chunk.chunk.pos.y);
		world_copy_chunk(sim->w, &sm->msg.chunk.chunk);

		sim->changed.chunks = true;
		break;
	case server_message_action:
		/* sim_copy_action(sim, &sm->msg.action.action); */
		break;
	case server_message_rem_action:
		sim_remove_action(sim, sm->msg.rem_action.id);
		break;
	case server_message_world_info:
		apply_world_info(sim, &sm->msg.world_info);
		break;
	case server_message_hello:
		sim->assigned_motivator = sm->msg.hello.alignment;
		break;
	}

	return ir_cont;
}

void
world_update(struct simulation *sim, struct net_ctx *nx)
{
	darr_for_each(nx->recvd, sim, world_apply_update);
	darr_clear(nx->recvd);
}
