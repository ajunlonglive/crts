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
		"set_action_target", kc_set_action_target,
		"resize_selection", kc_resize_selection,
		"undo_action", kc_undo_action,
		"exec_action", kc_exec_action,
		"toggle_help", kc_toggle_help,
		"set_curs_action_type", kc_set_curs_action_type,
		"debug_pathfind_toggle", kc_debug_pathfind_toggle,
		"debug_pathfind_place_point", kc_debug_pathfind_place_point,
		"", kc_macro,
	},
	[cslt_constants] = {
		"tile_deep_water", tile_deep_water,
		"tile_water", tile_water,
		"tile_wetland", tile_wetland,
		"tile_plain", tile_plain,
		"tile_forest", tile_forest,
		"tile_mountain", tile_mountain,
		"tile_peak", tile_peak,
		"tile_dirt", tile_dirt,
		"tile_forest_young", tile_forest_young,
		"tile_forest_old", tile_forest_old,
		"tile_wetland_forest_young", tile_wetland_forest_young,
		"tile_wetland_forest", tile_wetland_forest,
		"tile_wetland_forest_old", tile_wetland_forest_old,
		"tile_coral", tile_coral,
		"tile_stream", tile_stream,
		"tile_wood", tile_wood,
		"tile_stone", tile_stone,
		"tile_wood_floor", tile_wood_floor,
		"tile_rock_floor", tile_rock_floor,
		"tile_storehouse", tile_storehouse,
		"tile_farmland_empty", tile_farmland_empty,
		"tile_farmland_done", tile_farmland_done,
		"tile_burning", tile_burning,
		"tile_burnt", tile_burnt,
		"at_none", at_none,
		"at_move", at_move,
		"at_harvest", at_harvest,
		"at_build", at_build,
		"at_fight", at_fight,
		"at_carry", at_carry,
		"im_normal", im_normal,
		"im_cmd", im_cmd,
		"neutral", curs_act_neutral,
		"create", curs_act_create,
		"destroy", curs_act_destroy,
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
