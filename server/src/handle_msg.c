#include "sim/sim.h"
#include "handle_msg.h"
#include "util/log.h"
#include "net/wrapped_message.h"

void handle_msgs(struct simulation *sim)
{
	struct wrapped_message *wm;
	struct action *act;

	while (1) {
		wm = queue_pop(sim->inbound, 1);

		switch (wm->cm.type) {
		case client_message_poke:
			break;
		case client_message_chunk_req:
			break;
		case client_message_action:
			L("adding action");
			act = sim_add_act(sim, NULL);

			act->motivator = 1;
			act->type = ((struct cm_action *)wm->cm.update)->type;
			act->range = ((struct cm_action *)wm->cm.update)->range;

			action_inspect(act);
			break;
		}
	}
}

