#ifndef SHARED_OPENGL_MENU_H
#define SHARED_OPENGL_MENU_H

#include "shared/math/linalg.h"
#include "shared/types/geom.h"
#include "shared/ui/gl/window.h"

enum menu_align {
	menu_align_left,
	menu_align_right,
};

enum menu_theme_elems {
	menu_theme_elem_bar,
	menu_theme_elem_win,
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
	enum menu_theme_elems clr;
	float w;
	bool hovered;
};

#define MENU_TEXTBOX_BUF_LEN 255
struct menu_textbox_ctx {
	float min_w;
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
	bool something_was_hovered;

	bool initialized;
};

extern struct menu_ctx menu;

bool menu_setup(void);

void menu_textbox(struct menu_textbox_ctx *tctx);
bool menu_button(const char *str, enum menu_button_flags flags);
bool menu_button_c(struct menu_button_ctx *bctx);
bool menu_slider(struct menu_slider_ctx *slider_ctx, float *val);
void menu_printf(const char *fmt, ...) __attribute__ ((format(printf, 1, 2)));
uint32_t menu_rect(struct menu_rect *rect, enum menu_theme_elems clr);
void menu_cursor(const struct pointf *p, enum menu_theme_elems clr);
void menu_str(const char *str);
void menu_rect_str(struct menu_rect *rect,
	enum menu_theme_elems clr, enum menu_align, const char *str);
void menu_rect_printf(struct menu_rect *rect,
	enum menu_theme_elems clr, enum menu_align, const char *str,  ...) __attribute__ ((format(printf, 4, 5)));

bool menu_win(struct menu_win_ctx *win_ctx);
void menu_win_end(void);

void menu_align(float w);

void menu_newline(void);
void menu_goto_bottom_right(void);

void menu_set_scale(float new_scale);

void menu_begin(struct gl_win *win, float mousex, float mousey, bool clicked);
void menu_render(struct gl_win *win);
void menu_handle_input(uint8_t key);
#endif
