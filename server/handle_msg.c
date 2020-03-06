#ifndef CRTS_SERVER
#define CRTS_SERVER
#endif

#include "server/handle_msg.h"
#include "server/net/connection.h"
#include "server/net/wrapped_message.h"
#include "server/sim/action.h"
#include "server/sim/sim.h"
#include "server/sim/terrain.h"
#include "shared/messaging/server_message.h"
#include "shared/types/hash.h"
#include "shared/util/log.h"

static struct hash *motivators;

static void
handle_new_connection(struct simulation *sim, struct wrapped_message *wm)
{
	const size_t *motp;
	size_t mot;
	uint8_t uint8;
	struct server_message *sm;

	if ((motp = hash_get(motivators, &wm->cm.client_id)) == NULL) {
		mot = add_new_motivator(sim);
		hash_set(motivators, &wm->cm.client_id, mot);
	} else {
		mot = *motp;
	}

	uint8 = wm->cx->motivator = mot;

	sm = sm_create(server_message_hello, &uint8);
	sm->dest = wm->cx;

	queue_push(sim->outbound, sm);
}

void
handle_msgs_init(void)
{
	motivators = hash_init(32, 1, sizeof(uint32_t));
}

void
handle_msgs(struct simulation *sim)
{
	struct wrapped_message *wm;
	struct action *act;
	const struct chunk *ck;
	struct server_message *sm;
	struct ent *e;
	uint32_t id;

	while ((wm = queue_pop(sim->inbound)) != NULL) {
		if (wm->cx->new) {
			handle_new_connection(sim, wm);
			wm->cx->new = false;
		}

		switch (wm->cm.type) {
		case client_message_poke:
			break;
		case client_message_ent_req:
			id = ((struct cm_ent_req *)wm->cm.update)->id;
			if ((e = hdarr_get(sim->world->ents, &id)) != NULL) {
				queue_push(sim->outbound, sm_create(server_message_ent, e));
			}
			break;
		case client_message_chunk_req:
			L("got a chunk request ");
			ck = get_chunk(sim->world->chunks, &((struct cm_chunk_req *)wm->cm.update)->pos);
			L("retreived chunk @ %d, %d", ck->pos.x, ck->pos.y);
			sm = sm_create(server_message_chunk, ck);
			sm->dest = wm->cx;
			queue_push(sim->outbound, sm);

			break;
		case client_message_action:
			L("adding action ");
			act = &action_add(sim, NULL)->act;

			act->motivator = 1;
			act->type = ((struct cm_action *)wm->cm.update)->type;
			act->workers_requested = ((struct cm_action *)wm->cm.update)->workers;
			act->range = ((struct cm_action *)wm->cm.update)->range;
			act->tgt = ((struct cm_action *)wm->cm.update)->tgt;

			action_inspect(act);
			break;
		}
	}
}
