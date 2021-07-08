#ifndef CLIENT_UI_NCURSES_WINDOW_H
#define CLIENT_UI_NCURSES_WINDOW_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "shared/input/keyboard.h"
#include "shared/types/geom.h"

struct term_pixel {
	uint64_t attr;
	uint32_t clr;
	int16_t fg;
	int16_t bg;
	char c;
};

enum term_win_split {
	term_win_split_vertical,
	term_win_split_horizontal
};

struct term_win;

typedef void ((*term_key_cb)(void *ctx, uint8_t key, uint8_t mod, enum key_action action));
typedef void ((*term_mouse_cb)(void *ctx, float dx, float dy));

void term_setup(void);
void term_teardown(void);
uint32_t term_win_create_root(double split_pct, enum term_win_split split);
uint32_t term_win_create(uint32_t parent, double split_pct, enum term_win_split split);
void term_setup_color_pair(short f, short b, uint32_t id);
uint64_t term_attr_transform(uint8_t attr);
void term_commit_layout(void);
const struct rectangle *term_win_rect(uint32_t win);

void term_poll_events(void *ctx, term_key_cb key_cb, term_mouse_cb mouse_cb);
bool term_check_resize(void);
void term_swap_buffers(void);
void term_clear_attr(void);

void term_clear(void);
void term_write(uint32_t win_id, const struct point *p, char c);
void term_write_px(uint32_t win_id, const struct point *p, const struct term_pixel *px);
void term_write_str(uint32_t win_id, const struct point *p, const char *str);
uint32_t term_printf(uint32_t win_id, const struct point *p, const char *fmt, ...) __attribute__ ((format(printf, 3, 4)));
void term_clrtoeol(uint32_t win_id, const struct point *p);
#endif
