#include "posix.h"

#include <stdlib.h>

#include "client/input/keymap.h"
#include "shared/sim/action.h"
#include "shared/sim/chunk.h"
#include "shared/util/mem.h"

const struct cfg_lookup_table
	cmd_string_lookup_tables[cmd_string_lookup_table_count] = {
	[cslt_commands] = {
		"none", kc_none,
		"invalid", kc_invalid,
		"center_cursor", kc_center_cursor,
		"view_left", kc_view_left,
		"view_down", kc_view_down,
		"view_up", kc_view_up,
		"view_right", kc_view_right,
		"find", kc_find,
		"set_input_mode", kc_set_input_mode,
		"quit", kc_quit,
		"cursor_left", kc_cursor_left,
		"cursor_down", kc_cursor_down,
		"cursor_up", kc_cursor_up,
		"cursor_right", kc_cursor_right,
		"set_action_type", kc_set_action_type,
		"debug_pathfind_toggle", kc_debug_pathfind_toggle,
		"debug_pathfind_place_point", kc_debug_pathfind_place_point,
		"pause", kc_pause,
		"", kc_macro,
	},
	[cslt_constants] = {
		"im_normal", im_normal,
		"im_cmd", im_cmd,
		"neutral", act_neutral,
		"create", act_create,
		"destroy", act_destroy,
	},
};

const char *input_mode_names[input_mode_count] = {
	[im_normal] = "normal",
	[im_cmd]    = "command",
};

/* TODO: revisit keymap structure.  It can probably do with an overhaul */
void
keymap_init(struct keymap *km)
{
	km->map = z_calloc(ASCII_RANGE, sizeof(struct keymap));
}
