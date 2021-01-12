#include "posix.h"

#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#include "client/client.h"
#include "client/input/handler.h"
#include "client/input/helpers.h"
#include "client/input/move_handler.h"
#include "shared/util/log.h"
#include "shared/util/util.h"

long
client_get_num(struct client *cli, long def)
{
	return cli->num_override.override ? cli->num_override.val :
	       ((cli->num.len <= 0) ? def : strtol(cli->num.buf, NULL, 10));
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
	if (hbf->len >= INPUT_BUF_LEN - 1) {
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
		cli->changed.next_act = true;
	} else if (cli->resize.tmpcurs.x <= cli->resize.cntr.x) {
		cli->next_act.range.width = clamp(cli->resize.cntr.x - cli->resize.tmpcurs.x + 1, 1, ACTION_RANGE_MAX_W);
		cli->cursor.x = clamp(cli->resize.tmpcurs.x, cli->resize.cntr.x - ACTION_RANGE_MAX_W + 1, cli->resize.cntr.x);
		cli->changed.next_act = true;
	}

	if (cli->resize.tmpcurs.y > cli->resize.cntr.y) {
		cli->next_act.range.height = clamp(cli->resize.tmpcurs.y - cli->resize.cntr.y + 1, 1, ACTION_RANGE_MAX_H);
		cli->cursor.y = cli->resize.cntr.y;
		cli->changed.next_act = true;
	} else if (cli->resize.tmpcurs.y <= cli->resize.cntr.y) {
		cli->next_act.range.height = clamp(cli->resize.cntr.y - cli->resize.tmpcurs.y + 1, 1, ACTION_RANGE_MAX_H);
		cli->cursor.y = clamp(cli->resize.tmpcurs.y, cli->resize.cntr.y - ACTION_RANGE_MAX_H + 1, cli->resize.cntr.y);
		cli->changed.next_act = true;
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
void
client_reset_input(struct client *cli)
{
	memset(&cli->next_act, 0, sizeof(struct action));

	cli->next_act.range.width = 1;
	cli->next_act.range.height = 1;
}

void
client_init_view(struct client *cli, struct point *p)
{
	cli->view = *p;
	cli->cursor = (struct point) { 0, 0 };
	center_cursor(cli);
	cli->state |= csf_view_initialized;
}
