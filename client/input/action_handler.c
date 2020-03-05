#include "client/input/action_handler.h"
#include "client/input/handler.h"
#include "shared/messaging/client_message.h"
#include "shared/types/queue.h"
#include "shared/util/log.h"

void
set_action_type(struct hiface *hif)
{
	long id;

	if ((id = hiface_get_num(hif, 0)) >= action_type_count || id < 0) {
		return;
	}

	hif->next_act.type = id;
	hif->next_act_changed = true;
}

void
set_action_radius(struct hiface *hif)
{
	long r = hiface_get_num(hif, 1);

	hif->next_act.range.r = r > 0 ? r : 1;
	hif->next_act_changed = true;
}

void
set_action_target(struct hiface *hif)
{
	//long tgt = hiface_get_num(hif, 0);
	hif->next_act_changed = true;
}

void
exec_action(struct hiface *hif)
{
	if (hif->next_act.type == at_none) {
		return;
	}

	hif->next_act.range.center = point_add(&hif->view, &hif->cursor);
	hif->next_act.workers_requested = hiface_get_num(hif, 1);

	queue_push(hif->sim->outbound, cm_create(client_message_action, &hif->next_act));
}
