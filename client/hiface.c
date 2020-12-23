#include "posix.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "client/cfg/keymap.h"
#include "client/hiface.h"
#include "client/input/handler.h"
#include "client/net.h"
#include "shared/types/hdarr.h"
#include "shared/util/log.h"
#include "shared/util/util.h"

void
hiface_reset_input(struct hiface *hf)
{
	memset(&hf->next_act, 0, sizeof(struct action));

	hf->next_act.range.width = 1;
	hf->next_act.range.height = 1;
}

void
hiface_init(struct hiface *hf, struct c_simulation *sim)
{
	size_t i;

	hf->sim = sim;

	for (i = 0; i < input_mode_count; ++i) {
		keymap_init(&hf->km[i]);
	}

	hiface_reset_input(hf);
	hf->im = im_normal;
	hf->next_act.type = at_move;

#ifndef NDEBUG
	darr_init(&hf->debug_path.path_points, sizeof(struct point));
#endif
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

	if (hif->nx) {
		queue_msg(hif->nx, mt_action, &msg, hif->nx->cxs.cx_bits, msgf_forget);
	}
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
		.id = hif->next_act.id,
		.dat = {
			.add = {
				.tgt = hif->next_act.tgt,
				.type = hif->next_act.type,
				.range = hif->next_act.range
			},
		}
	};

	if (hif->nx) {
		queue_msg(hif->nx, mt_action, &msg, hif->nx->cxs.cx_bits, msgf_forget);
	}
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

void
check_selection_resize(struct hiface *hf)
{
	constrain_cursor(&hf->viewport, &hf->resize.tmpcurs);

	if (hf->resize.tmpcurs.x > hf->resize.cntr.x) {
		hf->next_act.range.width = clamp(hf->resize.tmpcurs.x - hf->resize.cntr.x + 1, 1, ACTION_RANGE_MAX_W);
		hf->cursor.x = hf->resize.cntr.x;
		hf->next_act_changed = true;
	} else if (hf->resize.tmpcurs.x <= hf->resize.cntr.x) {
		hf->next_act.range.width = clamp(hf->resize.cntr.x - hf->resize.tmpcurs.x + 1, 1, ACTION_RANGE_MAX_W);
		hf->cursor.x = clamp(hf->resize.tmpcurs.x, hf->resize.cntr.x - ACTION_RANGE_MAX_W + 1, hf->resize.cntr.x);
		hf->next_act_changed = true;
	}

	if (hf->resize.tmpcurs.y > hf->resize.cntr.y) {
		hf->next_act.range.height = clamp(hf->resize.tmpcurs.y - hf->resize.cntr.y + 1, 1, ACTION_RANGE_MAX_H);
		hf->cursor.y = hf->resize.cntr.y;
		hf->next_act_changed = true;
	} else if (hf->resize.tmpcurs.y <= hf->resize.cntr.y) {
		hf->next_act.range.height = clamp(hf->resize.cntr.y - hf->resize.tmpcurs.y + 1, 1, ACTION_RANGE_MAX_H);
		hf->cursor.y = clamp(hf->resize.tmpcurs.y, hf->resize.cntr.y - ACTION_RANGE_MAX_H + 1, hf->resize.cntr.y);
		hf->next_act_changed = true;
	}

	hf->resize.oldcurs = hf->cursor;

	constrain_cursor(&hf->viewport, &hf->cursor);
}

void
constrain_cursor(struct rectangle *ref, struct point *curs)
{
	if (curs->y <= 0) {
		curs->y = 1;
	} else if (curs->y >= ref->height) {
		curs->y = ref->height - 1;
	}

	if (curs->x <= 0) {
		curs->x = 1;
	} else if (curs->x >= ref->width) {
		curs->x = ref->width - 1;
	}
}

void
resize_selection_start(struct hiface *hf)
{
	if (!hf->resize.b) {
		hf->resize.tmpcurs = hf->resize.cntr = hf->cursor;
		hf->resize.b = true;
	}
}

void
resize_selection_stop(struct hiface *hf)
{
	if (hf->resize.b) {
		hf->cursor = hf->resize.oldcurs;
		hf->resize.b = false;
	}
}

void
move_viewport(struct hiface *hf, int32_t dx, int32_t dy)
{
	resize_selection_stop(hf);

	hf->view.x -= dx;
	hf->view.y -= dy;

	trigger_cmd_with_num(kc_cursor_right, hf, dx);
	trigger_cmd_with_num(kc_cursor_down, hf, dy);
}
