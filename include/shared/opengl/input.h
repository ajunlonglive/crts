#ifndef SHARED_OPENGL_INPUT_H
#define SHARED_OPENGL_INPUT_H
enum mouse_buttons {
	mb_1 = 1 << 0,
	mb_2 = 1 << 1,
	mb_3 = 1 << 2,
	mb_4 = 1 << 3,
	mb_5 = 1 << 4,
	mb_6 = 1 << 5,
	mb_7 = 1 << 6,
	mb_8 = 1 << 7,
};

enum modifier_types {
	mod_shift = 1 << 0,
	mod_ctrl  = 1 << 1,
};

struct gl_input {
	struct {
		double lx, ly, x, y, dx, dy, scroll;
		double cursx, cursy;
		bool still, init;
		uint8_t buttons, old_buttons;
	} mouse;
	struct {
		uint8_t held[0xff];
		uint8_t mod;
		bool flying;
	} keyboard;
};
#endif
