#ifndef CLIENT_UI_OPENGL_WINDOW_H
#define CLIENT_UI_OPENGL_WINDOW_H

#include <stdbool.h>
#include <stdint.h>

#define INPUT_KEY_BUF_MAX 8

typedef void ((*keyboard_oneshot_callback)(void *ctx, uint8_t mod, uint8_t key));

struct gl_win {
	uint32_t px_height, px_width, sc_height, sc_width;
	bool resized;

	struct {
		double lx, ly, x, y, dx, dy, scroll,
		       scaled_dx, scaled_dy;
		double cursx, cursy;
		bool still, init;
		uint8_t buttons, old_buttons;
	} mouse;

	struct {
		uint8_t mod;
		uint16_t held[INPUT_KEY_BUF_MAX];
		uint8_t held_len;
	} keyboard;

	keyboard_oneshot_callback keyboard_oneshot_callback;
};

struct gl_win *win_init(void *ctx);
void win_set_cursor_display(bool mode);
bool win_is_focused(void);
void win_terminate(void);
void win_poll_events(void);
void win_swap_buffers(void);
#endif
