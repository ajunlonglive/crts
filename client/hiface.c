#include <stdlib.h>

#include "client/cfg/keymap.h"
#include "client/hiface.h"
#include "client/net.h"
#include "shared/types/hdarr.h"

struct hiface *
hiface_init(struct simulation *sim)
{
	size_t i;
	struct hiface *hf = calloc(1, sizeof(struct hiface));

	hf->sim = sim;
	hf->im = im_normal;
	hf->next_act.range.r = 3;

	for (i = 0; i < input_mode_count; ++i) {
		keymap_init(&hf->km[i]);
	}

	return hf;
}

long
hiface_get_num(struct hiface *hif, long def)
{
	return hif->num_override.override ? hif->num_override.val :
	       ((hif->num.len <= 0) ? def : strtol(hif->num.buf, NULL, 10));
}

void
undo_action(struct hiface *hif)
{
	uint8_t na;
	struct action *act;

	if (hif->sim->action_history_len == 0) {
		return;
	}

	--hif->sim->action_history_len;

	na = hif->sim->action_history_order[hif->sim->action_history_len];
	act = &hif->sim->action_history[na];
	act->type = at_none;

	hif->next_act_changed = true;

	send_msg(hif->nx, client_message_action, act, 0);
}

void
commit_action(struct hiface *hif)
{
	if (hif->next_act.type == at_none) {
		return;
	}

	hif->next_act.id = (hif->action_seq++) % ACTION_HISTORY_SIZE;
	hif->sim->action_history[hif->next_act.id] = hif->next_act;

	hif->sim->action_history_order[hif->sim->action_history_len] = hif->next_act.id;

	/* TODO: relying on uint8_t overflow to keep index in bounds */
	++hif->sim->action_history_len;

	send_msg(hif->nx, client_message_action, &hif->next_act, 0);
}

void
override_num_arg(struct hiface *hf, long num)
{
	hf->num_override.override = true;
	hf->num_override.val = num;
}
