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
	if (cli->keymap_describe) {
		cli_describe(cli, kmc_sys, "end program");
		return;
	}

	cli->run = 0;
}

static void
set_input_mode(struct client *d)
{
	enum input_mode im = client_get_num(d, 0) % input_mode_count;

	if (d->keymap_describe) {
		cli_describe(d, kmc_sys, "enter %s mode", input_mode_names[im]);
	}

	d->im = im;
}

static void
toggle_help(struct client *cli)
{
	if (cli->keymap_describe) {
		cli_describe(cli, kmc_sys, "toggle help");
		return;
	}

	cli->state ^= csf_display_help;
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
	[kc_set_action_target]    = set_action_target,
	[kc_undo_action]          = undo_last_action,
	[kc_resize_selection]     = resize_selection,
	[kc_exec_action]          = exec_action,
	[kc_toggle_help]          = toggle_help,

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

static void
do_macro(struct client *cli, char *macro)
{
	size_t i, len = strlen(macro);
	struct keymap *mkm = &cli->keymaps[cli->im];

	//clib_clear(&cli->num);
	//clib_clear(&cli->cmd);

	for (i = 0; i < len; i++) {
		if ((mkm = handle_input(mkm, macro[i], cli)) == NULL) {
			mkm = &cli->keymaps[cli->im];
		}
	}
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
	if (cli->resize.b) {
		cli->resize.oldcurs = cli->cursor;
		cli->cursor = cli->resize.tmpcurs;
	}

	kc_funcs[kc](cli);

	if (cli->resize.b) {
		cli->resize.tmpcurs = cli->cursor;
		cli->cursor = cli->resize.oldcurs;
		check_selection_resize(cli);
	}

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
		/* L("node:expr:%d", node->val.expr.kc); */
		if (node->val.expr.argc) {
			trigger_cmd_with_num(node->val.expr.kc, cli, node->val.expr.argv[0]);
		} else {
			trigger_cmd(node->val.expr.kc, cli);
		}
		*mkm = &cli->keymaps[cli->im];
		break;
	case kcmnt_char:
		/* L("node:char:%d", node->val.c); */
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
	} else if (!cli->keymap_describe) {
		clib_append_char(&cli->cmd, k);
	}

	if (!km) {
		LOG_W("invalid macro");
		return NULL;
	}

	if (km->map[k].map) {
		return &km->map[k];
	} else if (km->map[k].cmd.nodes) {
		exec_macro(cli, &km->map[k].cmd);
		//clib_clear(&cli->num);
		//clib_clear(&cli->cmd);

		/* if (km->map[k].cmd == kc_macro) { */
		/* 	clib_clear(&cli->num); */
		/* 	do_macro(cli, km->map[k].strcmd); */
		/* } else { */
		/* 	trigger_cmd(km->map[k].cmd, cli); */
		/* } */
	}

	clib_clear(&cli->cmd);
	return NULL;
}

void
for_each_completion(struct keymap *km, void *ctx, for_each_completion_cb cb)
{
	unsigned k;

	for (k = 0; k < ASCII_RANGE; ++k) {
		if (km->map[k].map) {
			for_each_completion(&km->map[k], ctx, cb);
		} else if (km->map[k].cmd.nodes) {
			cb(ctx, &km->map[k]);
		}
	}
}

struct describe_completions_ctx {
	struct client *cli;
	struct client_buf *num;
	void *ctx;
	for_each_completion_cb cb;
};

static void
describe_completion(void *_ctx, struct keymap *km)
{
	struct describe_completions_ctx *ctx = _ctx;
	enum input_mode oim = ctx->cli->im;

	memset(ctx->cli->description, 0, KEYMAP_DESC_LEN);
	ctx->cli->desc_len = 0;

	do_macro(ctx->cli, km->trigger);

	strncpy(km->desc, ctx->cli->description, KEYMAP_DESC_LEN);
	ctx->cb(ctx->ctx, km);

	client_reset_input(ctx->cli);
	ctx->cli->im = oim;
	ctx->cli->num = *ctx->num;
}

void
describe_completions(struct client *cli, struct keymap *km,
	void *usr_ctx, for_each_completion_cb cb)
{

	struct client_buf nbuf = cli->num;
	struct client_buf cbuf = cli->cmd;
	struct action act = cli->next_act;

	struct describe_completions_ctx ctx = {
		.cli = cli,
		.ctx = usr_ctx,
		.cb = cb,
		.num = &nbuf,
	};

	cli->keymap_describe = true;

	for_each_completion(km, &ctx, describe_completion);

	cli->keymap_describe = false;

	cli->cmd = cbuf;
	cli->next_act = act;
}
