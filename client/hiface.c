#include "posix.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "client/cfg/keymap.h"
#include "client/hiface.h"
#include "client/net.h"
#include "shared/types/hdarr.h"
#include "shared/util/log.h"

void
hiface_reset_input(struct hiface *hf)
{
	memset(&hf->next_act, 0, sizeof(struct action));

	hf->next_act.range.width = 1;
	hf->next_act.range.height = 1;
}

struct hiface *
hiface_init(struct c_simulation *sim)
{
	size_t i;
	struct hiface *hf = calloc(1, sizeof(struct hiface));

	hf->sim = sim;

	for (i = 0; i < input_mode_count; ++i) {
		keymap_init(&hf->km[i]);
	}

	hf->im = im_select;
	hf->next_act.type = at_move;

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

	struct msg_action msg = {
		.mt = amt_del,
		.id = act->id, /* TODO we only need the id on del? */
	};

	queue_msg(hif->nx, mt_action, &msg, hif->nx->cxs.cx_bits, msgf_forget);
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

	struct msg_action msg = {
		.mt = amt_add,
		.dat = {
			.add = {
				.type = hif->next_act.type,
				.range = hif->next_act.range
			},
		}
	};

	queue_msg(hif->nx, mt_action, &msg, hif->nx->cxs.cx_bits, msgf_forget);
}

void
override_num_arg(struct hiface *hf, long num)
{
	hf->num_override.override = true;
	hf->num_override.val = num;
}

void
hf_describe(struct hiface *hf, enum keymap_category cat, char *desc, ...)
{
	va_list ap;

	if (KEYMAP_DESC_LEN - hf->desc_len <= 1) {
		return;
	} else if (hf->desc_len) {
		hf->description[hf->desc_len++] = ' ';
	} else {
		hf->description[hf->desc_len++] = cat;
	}

	va_start(ap, desc);
	hf->desc_len += vsnprintf(&hf->description[hf->desc_len],
		KEYMAP_DESC_LEN - hf->desc_len, desc, ap);
	va_end(ap);
}

void
hifb_append_char(struct hiface_buf *hbf, unsigned c)
{
	if (hbf->len >= HF_BUF_LEN - 1) {
		return;
	}

	hbf->buf[hbf->len] = c;
	hbf->buf[hbf->len + 1] = '\0';
	hbf->len++;
}

