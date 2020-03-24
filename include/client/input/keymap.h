#ifndef CLIENT_INPUT_KEYMAP_H
#define CLIENT_INPUT_KEYMAP_H

#define ASCII_RANGE 128
#define KEYMAP_MACRO_LEN 32

enum key_command {
	kc_none,
	kc_center,
	kc_macro,
	kc_invalid,
	kc_view_left,
	kc_view_down,
	kc_view_up,
	kc_view_right,
	kc_enter_selection_mode,
	kc_enter_normal_mode,
	kc_quit,
	kc_cursor_left,
	kc_cursor_down,
	kc_cursor_up,
	kc_cursor_right,
	kc_set_action_type,
	kc_set_action_target,
	kc_set_action_radius,
	kc_exec_action,
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
	im_none,
	im_invalid,
	input_mode_count = 2
};

struct keymap {
	enum key_command cmd;
	char strcmd[KEYMAP_MACRO_LEN];
	struct keymap *map;
};

void keymap_init(struct keymap *km);
#endif
