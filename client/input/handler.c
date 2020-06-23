#include "posix.h"

#include <stdlib.h>
#include <string.h>

#include "client/cfg/keymap.h"
#include "client/input/action_handler.h"
#include "client/input/handler.h"
#include "client/input/keymap.h"
#include "client/input/move_handler.h"
#include "shared/sim/action.h"
#include "shared/util/log.h"

static void
do_nothing(struct hiface *_)
{
}

static void
end_simulation(struct hiface *d)
{
	if (d->keymap_describe) {
		hf_describe(d, "end program");
		return;
	}

	d->sim->run = 0;
}

static void
set_input_mode_select(struct hiface *d)
{
	if (d->keymap_describe) {
		hf_describe(d, "set input mode select");
	}

	d->im = im_select;
}

static void
set_input_mode_normal(struct hiface *d)
{
	if (d->keymap_describe) {
		hf_describe(d, "set input mode normal");
	}

	d->im = im_normal;
}

static void
set_input_mode_resize(struct hiface *d)
{
	if (d->keymap_describe) {
		hf_describe(d, "set input mode resize");
	}

	d->im = im_resize;
}

static kc_func kc_funcs[key_command_count] = {
	[kc_none]                 = do_nothing,
	[kc_invalid]              = do_nothing,
	[kc_macro]                = do_nothing,
	[kc_center]               = center,
	[kc_center_cursor]        = center_cursor,
	[kc_view_up]              = view_up,
	[kc_view_down]            = view_down,
	[kc_view_left]            = view_left,
	[kc_view_right]           = view_right,
	[kc_find]                 = find,
	[kc_enter_selection_mode] = set_input_mode_select,
	[kc_enter_normal_mode]    = set_input_mode_normal,
	[kc_enter_resize_mode]    = set_input_mode_resize,
	[kc_quit]                 = end_simulation,
	[kc_cursor_up]            = cursor_up,
	[kc_cursor_down]          = cursor_down,
	[kc_cursor_left]          = cursor_left,
	[kc_cursor_right]         = cursor_right,
	[kc_set_action_type]      = set_action_type,
	[kc_toggle_action_flag]   = toggle_action_flag,
	[kc_set_action_height]    = set_action_height,
	[kc_action_height_grow]   = action_height_grow,
	[kc_action_height_shrink] = action_height_shrink,
	[kc_set_action_width]     = set_action_width,
	[kc_action_width_grow]    = action_width_grow,
	[kc_action_width_shrink]  = action_width_shrink,
	[kc_action_rect_rotate]   = action_rect_rotate,
	[kc_set_action_target]    = set_action_target,
	[kc_read_action_target]   = read_action_target,
	[kc_undo_action]          = undo_last_action,
	[kc_exec_action]          = exec_action,
	[kc_swap_cursor_with_source] = swap_cursor_with_source,
};

static void
hifb_clear(struct hiface_buf *buf)
{
	buf->len = 0;
	buf->buf[0] = '\0';
}

static void
do_macro(struct hiface *hif, char *macro)
{
	size_t i, len = strlen(macro);
	struct keymap *mkm = &hif->km[hif->im];

	//hifb_clear(&hif->num);
	//hifb_clear(&hif->cmd);

	for (i = 0; i < len; i++) {
		if ((mkm = handle_input(mkm, macro[i], hif)) == NULL) {
			mkm = &hif->km[hif->im];
		}
	}
}

void
trigger_cmd(kc_func func, struct hiface *hf)
{
	func(hf);

	hifb_clear(&hf->num);

	hf->num_override.override = false;
	hf->num_override.val = 0;
}

struct keymap *
handle_input(struct keymap *km, unsigned k, struct hiface *hif)
{
	if (k > ASCII_RANGE) {
		return NULL;
	}

	hif->input_changed = true;

	if (k >= '0' && k <= '9') {
		hifb_append_char(&hif->num, k);
		return km;
	} else if (!hif->keymap_describe) {
		hifb_append_char(&hif->cmd, k);
	}

	if (!km) {
		LOG_W("invalid macro");
		return NULL;
	}

	if (km->map[k].map) {
		return &km->map[k];
	} else if (km->map[k].cmd) {
		if (km->map[k].cmd == kc_macro) {
			hifb_clear(&hif->num);
			do_macro(hif, km->map[k].strcmd);
		} else {
			trigger_cmd(kc_funcs[km->map[k].cmd], hif);
		}
	}

	hifb_clear(&hif->cmd);
	return NULL;
}

void
for_each_completion(struct keymap *km, void *ctx, for_each_completion_cb cb)
{
	unsigned k;

	for (k = 0; k < ASCII_RANGE; ++k) {
		if (km->map[k].map) {
			for_each_completion(&km->map[k], ctx, cb);
		} else if (km->map[k].cmd) {
			cb(ctx, &km->map[k]);
		}
	}
}

struct describe_completions_ctx {
	struct hiface *hf;
	struct hiface_buf *num;
	void *ctx;
	for_each_completion_cb cb;
};

static void
describe_completion(void *_ctx, struct keymap *km)
{
	struct describe_completions_ctx *ctx = _ctx;
	enum input_mode oim = ctx->hf->im;

	memset(ctx->hf->description, 0, KEYMAP_DESC_LEN);
	ctx->hf->desc_len = 0;

	do_macro(ctx->hf, km->trigger);

	L("%s -> %s", km->trigger, ctx->hf->description);
	strncpy(km->desc, ctx->hf->description, KEYMAP_DESC_LEN);
	ctx->cb(ctx->ctx, km);

	hiface_reset_input(ctx->hf);
	ctx->hf->im = oim;
	ctx->hf->num = *ctx->num;
}

void
describe_completions(struct hiface *hf, struct keymap *km,
	void *usr_ctx, for_each_completion_cb cb)
{

	struct hiface_buf nbuf = hf->num;
	struct hiface_buf cbuf = hf->cmd;
	struct action act = hf->next_act;

	struct describe_completions_ctx ctx = {
		.hf = hf,
		.ctx = usr_ctx,
		.cb = cb,
		.num = &nbuf,
	};

	hf->keymap_describe = true;

	for_each_completion(km, &ctx, describe_completion);

	hf->keymap_describe = false;

	hf->cmd = cbuf;
	hf->next_act = act;
}
