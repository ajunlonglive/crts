#ifndef CLIENT_UI_OPENGL_WINDOW_H
#define CLIENT_UI_OPENGL_WINDOW_H

#include <stdbool.h>
#include <stdint.h>

#define GL_WIN_MAX_HELD_KEYS 8

typedef void ((*gl_key_input_callback)(void *ctx, uint8_t mod, uint8_t key, uint8_t action));

struct gl_win {
	uint32_t px_height, px_width, sc_height, sc_width;
	bool resized;

	struct {
		double lx, ly, x, y, dx, dy, scroll;
		bool still, init;
		uint8_t buttons, old_buttons;
	} mouse;

	struct {
		uint8_t mod;
		uint16_t held[GL_WIN_MAX_HELD_KEYS];
		uint8_t held_len;
	} keyboard;

	gl_key_input_callback key_input_callback;
};

struct gl_win *gl_win_init(void *ctx);
void gl_win_set_cursor_display(bool mode);
bool gl_win_is_focused(void);
void gl_win_terminate(void);
void gl_win_poll_events(void);
void gl_win_swap_buffers(void);
#endif
