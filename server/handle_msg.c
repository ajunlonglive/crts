#ifndef CRTS_SERVER
#define CRTS_SERVER
#endif

#include "server/handle_msg.h"
#include "server/net/wrapped_message.h"
#include "server/sim/action.h"
#include "server/sim/sim.h"
#include "server/sim/terrain.h"
#include "shared/messaging/server_message.h"
#include "shared/util/log.h"

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
