#include "posix.h"

#include <stdlib.h>
#include <string.h>

#include "client/cfg/keymap.h"
#include "client/input/action_handler.h"
#include "client/input/cmdline.h"
#include "client/input/handler.h"
#include "client/input/helpers.h"
#include "client/input/keymap.h"
#include "client/input/move_handler.h"
#include "shared/sim/action.h"
#include "shared/util/log.h"

#ifndef NDEBUG
#include "client/input/debug.h"
#endif

static void
do_nothing(struct client *_)
{
}

static void
end_simulation(struct client *cli)
{
	cli->run = 0;
}

static void
pause_simulation(struct client *cli)
{
	cli->state ^= csf_paused;

	enum server_cmd cmd;
	if (cli->state & csf_paused) {
		cmd = server_cmd_pause;
	} else {
		cmd = server_cmd_unpause;
	}

	msgr_queue(cli->msgr, mt_server_cmd, &(struct msg_server_cmd) {
		.cmd = cmd,
	}, 0, priority_normal);
}

static void
set_input_mode(struct client *d)
{
	enum input_mode im = client_get_num(d, 0) % input_mode_count;

	d->im = im;
}

static kc_func kc_funcs[key_command_count] = {
	[kc_none]                 = do_nothing,
	[kc_invalid]              = do_nothing,
	[kc_macro]                = do_nothing,
	[kc_center_cursor]        = center_cursor,
	[kc_view_up]              = view_up,
	[kc_view_down]            = view_down,
	[kc_view_left]            = view_left,
	[kc_view_right]           = view_right,
	[kc_find]                 = find,
	[kc_set_input_mode]       = set_input_mode,
	[kc_quit]                 = end_simulation,
	[kc_cursor_up]            = cursor_up,
	[kc_cursor_down]          = cursor_down,
	[kc_cursor_left]          = cursor_left,
	[kc_cursor_right]         = cursor_right,
	[kc_set_action_type]      = set_action_type,
	[kc_pause]                = pause_simulation,

#ifndef NDEBUG
	[kc_debug_pathfind_toggle] = debug_pathfind_toggle,
	[kc_debug_pathfind_place_point] = debug_pathfind_place_point,
#else
	[kc_debug_pathfind_toggle] = do_nothing,
	[kc_debug_pathfind_place_point] = do_nothing,
#endif
};

static void
clib_clear(struct client_buf *buf)
{
	buf->len = 0;
	buf->buf[0] = '\0';
}

void
trigger_cmd_with_num(enum key_command kc, struct client *cli, int32_t val)
{
	override_num_arg(cli, val);
	trigger_cmd(kc, cli);
}

void
trigger_cmd(enum key_command kc, struct client *cli)
{
	kc_funcs[kc](cli);

	clib_clear(&cli->num);

	cli->num_override.override = false;
	cli->num_override.val = 0;
}

static void exec_node(struct client *cli, struct keymap **mkm, struct kc_node *node);

static void
exec_macro(struct client *cli, struct kc_macro *macro)
{
	struct keymap *mkm = &cli->keymaps[cli->im];
	uint8_t i;
	for (i = 0; i < macro->nodes; ++i) {
		exec_node(cli, &mkm, &macro->node[i]);
	}
}

static void
exec_node(struct client *cli, struct keymap **mkm, struct kc_node *node)
{
	switch (node->type) {
	case kcmnt_expr:
		/* L(log_misc, "node:expr:%d", node->val.expr.kc); */
		if (node->val.expr.argc) {
			trigger_cmd_with_num(node->val.expr.kc, cli, node->val.expr.argv[0]);
		} else {
			trigger_cmd(node->val.expr.kc, cli);
		}
		*mkm = &cli->keymaps[cli->im];
		break;
	case kcmnt_char:
		/* L(log_misc, "node:char:%d", node->val.c); */
		if ((*mkm)->map[(uint8_t)node->val.c].map) {
			*mkm = &(*mkm)->map[(uint8_t)node->val.c];
		} else {
			exec_macro(cli, &(*mkm)->map[(uint8_t)node->val.c].cmd);
			*mkm = &cli->keymaps[cli->im];
		}
		break;
	}
}

struct keymap *
handle_input(struct keymap *km, unsigned k, struct client *cli)
{
	if (k > ASCII_RANGE) {
		return NULL;
	} else if (cli->im == im_cmd) {
		parse_cmd_input(cli, k);
		return NULL;
	}

	cli->changed.input = true;

	if (k >= '0' && k <= '9') {
		clib_append_char(&cli->num, k);
		return km;
	}

	if (!km) {
		LOG_W(log_misc, "invalid macro");
		return NULL;
	}

	if (km->map[k].map) {
		return &km->map[k];
	} else if (km->map[k].cmd.nodes) {
		exec_macro(cli, &km->map[k].cmd);
	}

	clib_clear(&cli->cmd);
	return NULL;
}
