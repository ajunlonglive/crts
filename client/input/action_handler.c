#include "client/input/action_handler.h"
#include "client/input/handler.h"
#include "shared/messaging/client_message.h"
#include "shared/types/queue.h"
#include "shared/util/log.h"

void
action_move(struct hiface *hif)
{
	struct client_message *cm;
	struct action move = {
		.type = at_move,
		.workers.requested = hiface_get_num(hif, 1),
		.range = {
			.center = point_add(&hif->view, &hif->cursor),
			.r = 5
		}
	};

	cm = cm_create(client_message_action, &move);
	queue_push(hif->sim->outbound, cm);
}

void
action_harvest(struct hiface *hif)
{
	struct client_message *cm;
	struct action act = {
		.type = at_harvest,
		.workers.requested = hiface_get_num(hif, 1),
		.range = {
			.center = point_add(&hif->view, &hif->cursor),
			.r = 5
		}
	};

	cm = cm_create(client_message_action, &act);
	queue_push(hif->sim->outbound, cm);
}
