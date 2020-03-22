#include "client/input/action_handler.h"
#include "client/input/handler.h"
#include "client/net.h"
#include "shared/constants/globals.h"
#include "shared/messaging/client_message.h"
#include "shared/util/log.h"

/* When setting this number as the action type, the bldg_rotate bit will be
 * flipped in the current target instad */
#define MAGIC_ROTATE_NUMBER 64

static void
set_action_target_int(struct hiface *hif, long tgt)
{
	switch (hif->next_act.type) {
	case at_build:
		if (tgt < 0) {
			tgt = hif->next_act.tgt + 2;
		} else if (tgt == MAGIC_ROTATE_NUMBER) {
			tgt = hif->next_act.tgt ^ bldg_rotate;
		}

		tgt %= buildings_count;
		break;
	case at_harvest:
		if (tgt < 0) {
			tgt = hif->next_act.tgt + 1;
		}

		tgt %= tile_count;

		while (!gcfg.tiles[tgt].hardness) {
			tgt = (tgt + 1) % tile_count;
		}

		break;
	default:
		return;
		break;
	}

	if (hif->next_act.tgt == tgt) {
		return;
	}

	hif->next_act.tgt = tgt;
	hif->next_act_changed = true;
}


void
set_action_type(struct hiface *hif)
{
	long id;

	if ((id = hiface_get_num(hif, 0)) >= action_type_count || id < 0) {
		return;
	}

	hif->next_act.type = id;
	set_action_target_int(hif, 0);
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
	long tgt = hiface_get_num(hif, -1);

	set_action_target_int(hif, tgt);
}

void
exec_action(struct hiface *hif)
{
	if (hif->next_act.type == at_none) {
		return;
	}

	hif->next_act.range.center = point_add(&hif->view, &hif->cursor);
	hif->next_act.workers_requested = hiface_get_num(hif, 1);

	send_msg(hif->nx, client_message_action, &hif->next_act, 0);
}
