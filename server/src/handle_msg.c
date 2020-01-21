#include "sim/sim.h"
#include "handle_msg.h"
#include "util/log.h"
#include "net/wrapped_message.h"
#include "messaging/server_message.h"
#include "sim/terrain.h"

void
handle_msgs(struct simulation *sim)
{
	struct wrapped_message *wm;
	struct action *act;
	const struct chunk *ck;
	struct server_message *sm;

	while ((wm = queue_pop(sim->inbound)) != NULL) {

		switch (wm->cm.type) {
		case client_message_poke:
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
			act = &sim_add_act(sim, NULL)->act;

			act->motivator = 1;
			act->type = ((struct cm_action *)wm->cm.update)->type;
			act->range = ((struct cm_action *)wm->cm.update)->range;

			action_inspect(act);
			break;
		}
	}
}
