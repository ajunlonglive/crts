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
	return (hif->num.len <= 0) ? def : strtol(hif->num.buf, NULL, 10);
}

void
commit_action(struct hiface *hif)
{
	if (hif->next_act.type == at_none) {
		return;
	}

	hif->next_act.id = (hif->action_seq++) % ACTION_HISTORY_SIZE;
	hif->sim->action_history[hif->next_act.id] = hif->next_act;

	send_msg(hif->nx, client_message_action, &hif->next_act, 0);
}
