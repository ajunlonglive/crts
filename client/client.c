#include "posix.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "client/cfg/keymap.h"
#include "client/client.h"
#include "client/input/handler.h"
/* #include "client/net.h" */
#include "shared/types/hdarr.h"
#include "shared/util/log.h"
#include "shared/util/util.h"

void
client_reset_input(struct client *cli)
{
	memset(&cli->next_act, 0, sizeof(struct action));

	cli->next_act.range.width = 1;
	cli->next_act.range.height = 1;
}

void
client_init(struct client *cli, struct c_simulation *sim)
{
	size_t i;

	cli->sim = sim;

	for (i = 0; i < input_mode_count; ++i) {
		keymap_init(&cli->km[i]);
	}

	client_reset_input(cli);
	cli->im = im_normal;
	cli->next_act.type = at_move;

#ifndef NDEBUG
	darr_init(&cli->debug_path.path_points, sizeof(struct point));
#endif
}

long
client_get_num(struct client *cli, long def)
{
	return cli->num_override.override ? cli->num_override.val :
	       ((cli->num.len <= 0) ? def : strtol(cli->num.buf, NULL, 10));
}

void
undo_action(struct client *cli)
{
	uint8_t na;
	struct action *act;

	if (cli->sim->action_history_len == 0) {
		return;
	}

	--cli->sim->action_history_len;

	na = cli->sim->action_history_order[cli->sim->action_history_len];
	act = &cli->sim->action_history[na];
	act->type = at_none;

	cli->next_act_changed = true;

	struct msg_action msg = {
		.mt = amt_del,
		.id = act->id, /* TODO we only need the id on del? */
	};

	msgr_queue(cli->msgr, mt_action, &msg, 0x1);
}

void
commit_action(struct client *cli)
{
	if (cli->next_act.type == at_none) {
		return;
	}

	cli->next_act.id = (cli->action_seq++) % ACTION_HISTORY_SIZE;
	cli->sim->action_history[cli->next_act.id] = cli->next_act;

	cli->sim->action_history_order[cli->sim->action_history_len] = cli->next_act.id;

	/* TODO: relying on uint8_t overflow to keep index in bounds */
	++cli->sim->action_history_len;

	struct msg_action msg = {
		.mt = amt_add,
		.id = cli->next_act.id,
		.dat = {
			.add = {
				.tgt = cli->next_act.tgt,
				.type = cli->next_act.type,
				.range = cli->next_act.range
			},
		}
	};

	msgr_queue(cli->msgr, mt_action, &msg, 0x1);
}

void
override_num_arg(struct client *cli, long num)
{
	cli->num_override.override = true;
	cli->num_override.val = num;
}

void
cli_describe(struct client *cli, enum keymap_category cat, char *desc, ...)
{
	va_list ap;

	if (KEYMAP_DESC_LEN - cli->desc_len <= 1) {
		return;
	} else if (cli->desc_len) {
		cli->description[cli->desc_len++] = ' ';
	} else {
		cli->description[cli->desc_len++] = cat;
	}

	va_start(ap, desc);
	cli->desc_len += vsnprintf(&cli->description[cli->desc_len],
		KEYMAP_DESC_LEN - cli->desc_len, desc, ap);
	va_end(ap);
}

void
clib_append_char(struct client_buf *hbf, unsigned c)
{
	if (hbf->len >= HF_BUF_LEN - 1) {
		return;
	}

	hbf->buf[hbf->len] = c;
	hbf->buf[hbf->len + 1] = '\0';
	hbf->len++;
}

void
check_selection_resize(struct client *cli)
{
	constrain_cursor(&cli->viewport, &cli->resize.tmpcurs);

	if (cli->resize.tmpcurs.x > cli->resize.cntr.x) {
		cli->next_act.range.width = clamp(cli->resize.tmpcurs.x - cli->resize.cntr.x + 1, 1, ACTION_RANGE_MAX_W);
		cli->cursor.x = cli->resize.cntr.x;
		cli->next_act_changed = true;
	} else if (cli->resize.tmpcurs.x <= cli->resize.cntr.x) {
		cli->next_act.range.width = clamp(cli->resize.cntr.x - cli->resize.tmpcurs.x + 1, 1, ACTION_RANGE_MAX_W);
		cli->cursor.x = clamp(cli->resize.tmpcurs.x, cli->resize.cntr.x - ACTION_RANGE_MAX_W + 1, cli->resize.cntr.x);
		cli->next_act_changed = true;
	}

	if (cli->resize.tmpcurs.y > cli->resize.cntr.y) {
		cli->next_act.range.height = clamp(cli->resize.tmpcurs.y - cli->resize.cntr.y + 1, 1, ACTION_RANGE_MAX_H);
		cli->cursor.y = cli->resize.cntr.y;
		cli->next_act_changed = true;
	} else if (cli->resize.tmpcurs.y <= cli->resize.cntr.y) {
		cli->next_act.range.height = clamp(cli->resize.cntr.y - cli->resize.tmpcurs.y + 1, 1, ACTION_RANGE_MAX_H);
		cli->cursor.y = clamp(cli->resize.tmpcurs.y, cli->resize.cntr.y - ACTION_RANGE_MAX_H + 1, cli->resize.cntr.y);
		cli->next_act_changed = true;
	}

	cli->resize.oldcurs = cli->cursor;

	constrain_cursor(&cli->viewport, &cli->cursor);
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
resize_selection_start(struct client *cli)
{
	if (!cli->resize.b) {
		cli->resize.tmpcurs = cli->resize.cntr = cli->cursor;
		cli->resize.b = true;
	}
}

void
resize_selection_stop(struct client *cli)
{
	if (cli->resize.b) {
		cli->cursor = cli->resize.oldcurs;
		cli->resize.b = false;
	}
}

void
move_viewport(struct client *cli, int32_t dx, int32_t dy)
{
	resize_selection_stop(cli);

	cli->view.x -= dx;
	cli->view.y -= dy;

	trigger_cmd_with_num(kc_cursor_right, cli, dx);
	trigger_cmd_with_num(kc_cursor_down, cli, dy);
}
