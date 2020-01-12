#include "handler.h"
#include "messaging/client_message.h"
#include "util/log.h"
#include "types/queue.h"

void create_move_action(struct hiface *hif)
{
	struct client_message *cm;
	struct action move = {
		.type = at_move,
		.range = {
			.center = point_add(&hif->view, &hif->cursor),
			.r = 5
		}
	};

	cm = cm_create(client_message_action, &move);
	queue_push(hif->sim->outbound, cm);
}
