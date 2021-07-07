#ifndef SHARED_INPUT_KEYBOARD_H
#define SHARED_INPUT_KEYBOARD_H

enum special_keycodes {
	skc_up = 128,
	skc_down,
	skc_left,
	skc_right,
	skc_f1,
	skc_f2,
	skc_f3,
	skc_f4,
	skc_f5,
	skc_f6,
	skc_f7,
	skc_f8,
	skc_f9,
	skc_f10,
	skc_f11,
	skc_f12,
	skc_home,
	skc_end,
	skc_pgup,
	skc_pgdn,
	skc_mb1,
	skc_mb2,
	skc_mb3,
};

enum modifier_types {
	mod_shift = 1 << 0,
	mod_ctrl  = 1 << 1,
};

enum key_action {
	key_action_oneshot,
	key_action_press,
	key_action_release,
};
#endif
