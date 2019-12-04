#include "sim/sim.h"
#include "handle_msg.h"
#include "util/log.h"
#include "net/wrapped_message.h"

void handle_msgs(struct simulation *sim)
{
	struct wrapped_message *wm;

	while (1) {
		L("popping from %p", sim->inbound);
		wm = queue_pop(sim->inbound, 1);

		switch (wm->cm.type) {
		case client_message_poke:
			break;
		case client_message_chunk_req:
			break;
		case client_message_action:
			sim_add_act(sim, wm->cm.update);
			break;
		}
	}
}

