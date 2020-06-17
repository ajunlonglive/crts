#include "posix.h"

#include <stdlib.h>
#include <string.h>

#include "client/cfg/keymap.h"
#include "client/input/action_handler.h"
#include "client/input/handler.h"
#include "client/input/keymap.h"
#include "client/input/move_handler.h"
#include "shared/util/log.h"

static void
do_nothing(struct hiface *_)
{
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
	[kc_quit]                 = end_simulation,
	[kc_cursor_up]            = cursor_up,
	[kc_cursor_down]          = cursor_down,
	[kc_cursor_left]          = cursor_left,
	[kc_cursor_right]         = cursor_right,
	[kc_set_action_type]      = set_action_type,
	[kc_toggle_action_flag]   = toggle_action_flag,
	[kc_set_action_radius]    = set_action_radius,
	[kc_set_action_source]    = set_action_source,
	[kc_action_radius_expand] = action_radius_expand,
	[kc_action_radius_shrink] = action_radius_shrink,
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
hifb_append_char(struct hiface_buf *hbf, unsigned c)
{
	if (hbf->len >= sizeof(hbf->buf) - 1) {
		return;
	}

	hbf->buf[hbf->len] = c;
	hbf->buf[hbf->len + 1] = '\0';
	hbf->len++;
}

static void
do_macro(struct hiface *hif, struct keymap *km)
{
	size_t i, len = strlen(km->strcmd);
	struct keymap *mkm = &hif->km[hif->im];

	for (i = 0; i < len; i++) {
		if ((mkm = handle_input(mkm, km->strcmd[i], hif)) == NULL) {
			mkm = &hif->km[hif->im];
		}
	}
}

void
trigger_cmd(kc_func func, struct hiface *hf)
{
	func(hf);

	hifb_clear(&hf->num);
	hifb_clear(&hf->cmd);
	hf->num_override.override = false;
	hf->num_override.val = 0;
}

struct keymap *
handle_input(struct keymap *km, unsigned k, struct hiface *hif)
{
	if (k > ASCII_RANGE) {
		return NULL;
	}

	if (k >= '0' && k <= '9') {
		hifb_append_char(&hif->num, k);
		return km;
	} else {
		hifb_append_char(&hif->cmd, k);
	}

	km = &km->map[k];

	if (km->map == NULL) {
		if (km->cmd == kc_macro) {
			do_macro(hif, km);
		} else {
			trigger_cmd(kc_funcs[km->cmd], hif);
		}

		km = NULL;
	}

	return km;
}
