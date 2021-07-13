#ifndef SHARED_OPENGL_MENU_H
#define SHARED_OPENGL_MENU_H

#include "shared/math/linalg.h"
#include "shared/ui/gl/window.h"

enum menu_align {
	menu_align_left,
	menu_align_right,
};

enum menu_theme_elems {
	menu_theme_elem_win,
	menu_theme_elem_bar,
	menu_theme_elem_bar_hover,
	menu_theme_elem_bar_accent,
	menu_theme_elem_bar_accent2,
	menu_theme_elem_bar_active,
	menu_theme_elem_disabled,
	menu_theme_elem_fg,
	menu_theme_elem_count,
};

typedef vec4 menu_theme_definition[menu_theme_elem_count];

struct menu_rect { float x, y, h, w; };

struct menu_win_ctx {
	const char *title;
	float x, y, h, w;
	bool dragging, hidden;
	uint32_t rect_bar, rect_win;
};

struct menu_slider_ctx {
	float min, max, step, w;
	float pos, steps;
	bool dragging, init;
	const char *label, *unit;
};

typedef void ((*menu_generic_cb)(void));

enum menu_button_flags {
	menu_button_flag_disabled = 1 << 0,
};

struct menu_button_ctx {
	const char *str;
	menu_generic_cb hover_cb, click_cb;
	enum menu_button_flags flags;
	float w;
	bool hovered;
};

#define MENU_TEXTBOX_BUF_LEN 255
struct menu_textbox_ctx {
	char buf[MENU_TEXTBOX_BUF_LEN];
	uint32_t bufi;
};

struct menu_ctx {
	menu_theme_definition theme;
	float button_pad;
	bool center;
	float linesep;
	float scale, new_scale;
	struct gl_win *gl_win;

	struct menu_win_ctx *win;
	struct menu_textbox_ctx *textbox;
	uint32_t textbox_cursor, textbox_len;

	float x, y;

	float mousex, mousey, mousedx, mousedy;
	bool clicked, released, held, scale_changed;
};

bool menu_setup(struct menu_ctx *ctx);

void menu_textbox(struct menu_ctx *ctx, struct menu_textbox_ctx *tctx);
bool menu_button(struct menu_ctx *ctx, const char *str, enum menu_button_flags flags);
bool menu_button_c(struct menu_ctx *ctx, struct menu_button_ctx *bctx);
bool menu_slider(struct menu_ctx *ctx, struct menu_slider_ctx *slider_ctx, float *val);
void menu_printf(struct menu_ctx *ctx, const char *fmt, ...) __attribute__ ((format(printf, 2, 3)));
uint32_t menu_rect(struct menu_ctx *ctx, struct menu_rect *rect, enum menu_theme_elems clr);
void menu_str(struct menu_ctx *ctx, const char *str);
void menu_rect_str(struct menu_ctx *ctx, struct menu_rect *rect,
	enum menu_theme_elems clr, enum menu_align, const char *str);
void menu_rect_printf(struct menu_ctx *ctx, struct menu_rect *rect,
	enum menu_theme_elems clr, enum menu_align, const char *str,  ...) __attribute__ ((format(printf, 5, 6)));

bool menu_win(struct menu_ctx *ctx, struct menu_win_ctx *win_ctx);
void menu_win_end(struct menu_ctx *ctx);

void menu_newline(struct menu_ctx *ctx);
void menu_goto_bottom_right(struct menu_ctx *ctx);

void menu_set_scale(struct menu_ctx *ctx, float new_scale);

void menu_begin(struct menu_ctx *ctx, struct gl_win *win, float mousex, float mousey, bool clicked);
void menu_render(struct menu_ctx *ctx, struct gl_win *win);
void menu_handle_input(struct menu_ctx *ctx, uint8_t key);
#endif
