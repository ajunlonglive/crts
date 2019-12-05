#ifndef __KEYCMD_H
#define __KEYCMD_H
enum key_command {
	kc_none = 0,
	kc_invalid,
	kc_view_left,
	kc_view_down,
	kc_view_up,
	kc_view_right,
	kc_enter_selection_mode,
	kc_quit,
	kc_cursor_left,
	kc_cursor_down,
	kc_cursor_up,
	kc_cursor_right,
	kc_create_move_action,
};

#define KEY_COMMANDS 13
#endif
