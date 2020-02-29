#include "client/input/action_handler.h"
#include "client/input/handler.h"
#include "shared/messaging/client_message.h"
#include "shared/types/queue.h"
#include "shared/util/log.h"

static void
make_action(struct hiface *hif, enum action_type type)
{
	struct action act = {
		.type = type,
		.workers_requested = hiface_get_num(hif, 1),
		.range = {
			.center = point_add(&hif->view, &hif->cursor),
			.r = 5
		}
	};
	struct client_message *cm;

	cm = cm_create(client_message_action, &act);
	queue_push(hif->sim->outbound, cm);
}

void
action_move(struct hiface *hif)
{
	make_action(hif, at_move);
}

void
action_harvest(struct hiface *hif)
{
	make_action(hif, at_harvest);
}

void
action_build(struct hiface *hif)
{
	make_action(hif, at_build);
}
