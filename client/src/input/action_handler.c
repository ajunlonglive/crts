#include "handler.h"
#include "messaging/client_message.h"
#include "util/log.h"
#include "types/queue.h"

void create_move_action(void *d)
{
	struct display *disp = d;
	struct client_message *cm;
	struct action move = {
		.type = at_move,
		.range = {
			.center = point_add(&disp->view, &disp->cursor),
			.r = 1
		}
	};

	L("sending action");
	action_inspect(&move);

	cm = cm_create(client_message_action, &move);
	queue_push(disp->sim->outbound, cm);
}
