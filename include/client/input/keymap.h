#ifndef CLIENT_INPUT_KEYMAP_H
#define CLIENT_INPUT_KEYMAP_H

#define ASCII_RANGE 128
#define KEYMAP_MACRO_LEN 32
#define KEYMAP_DESC_LEN 64

#include <stddef.h>

enum key_command {
	kc_none,
	kc_center,
	kc_center_cursor,
	kc_macro,
	kc_invalid,
	kc_view_left,
	kc_view_down,
	kc_view_up,
	kc_view_right,
	kc_find,
	kc_enter_selection_mode,
	kc_enter_normal_mode,
	kc_enter_resize_mode,
	kc_quit,
	kc_cursor_left,
	kc_cursor_down,
	kc_cursor_up,
	kc_cursor_right,
	kc_set_action_type,
	kc_set_action_target,
	kc_toggle_action_flag,
	kc_read_action_target,
	kc_undo_action,
	kc_swap_cursor_with_source,
	kc_set_action_height,
	kc_action_height_grow,
	kc_action_height_shrink,
	kc_set_action_width,
	kc_action_width_grow,
	kc_action_width_shrink,
	kc_action_rect_rotate,
	kc_exec_action,
	kc_toggle_help,
	key_command_count
};

enum special_keycodes {
	skc_up = 1,
	skc_down = 2,
	skc_left = 3,
	skc_right = 4,
};

enum input_mode {
	im_normal,
	im_select,
	im_resize,
	im_none,
	im_invalid,
	input_mode_count = 3
};

extern const char *input_mode_names[input_mode_count];

struct keymap {
	char trigger[KEYMAP_MACRO_LEN];
	char strcmd[KEYMAP_MACRO_LEN];
	char desc[KEYMAP_DESC_LEN + 1];
	struct keymap *map;
	enum key_command cmd;
};

void keymap_init(struct keymap *km);
#endif
