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
move_viewport(struct client *cli, int32_t dx, int32_t dy)
{
	cli->view.x -= dx;
	cli->view.y -= dy;

	trigger_cmd_with_num(kc_cursor_right, cli, dx);
	trigger_cmd_with_num(kc_cursor_down, cli, dy);
}

void
client_reset_input(struct client *cli)
{
	// TODO
}

void
client_init_view(struct client *cli, struct point *p)
{
	cli->view = *p;
	cli->cursor = (struct point) { 0, 0 };
	center_cursor(cli);
	cli->state |= csf_view_initialized;
}
