#ifndef SHARED_OPENGL_MENU_H
#define SHARED_OPENGL_MENU_H

#include "shared/math/linalg.h"
#include "shared/opengl/window.h"

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
};

struct menu_ctx {
	menu_theme_definition theme;
	float scale, new_scale;
	struct gl_win *gl_win;

	struct menu_win_ctx *win;

	float x, y;

	float mousex, mousey, mousedx, mousedy;
	bool clicked, released, held, scale_changed;
};

enum menu_button_flags {
	menu_button_flag_disabled = 1 << 0,
};

bool menu_setup(struct menu_ctx *ctx);

bool menu_button(struct menu_ctx *ctx, const char *str, enum menu_button_flags flags);
bool menu_slider(struct menu_ctx *ctx, struct menu_slider_ctx *slider_ctx, float *val);
void menu_printf(struct menu_ctx *ctx, const char *fmt, ...);
uint32_t menu_rect(struct menu_ctx *ctx, struct menu_rect *rect, enum menu_theme_elems clr);
void menu_str(struct menu_ctx *ctx, const char *str);

bool menu_win(struct menu_ctx *ctx, struct menu_win_ctx *win_ctx);
void menu_win_end(struct menu_ctx *ctx);

void menu_newline(struct menu_ctx *ctx);
void menu_goto_bottom_right(struct menu_ctx *ctx);

void menu_set_scale(struct menu_ctx *ctx, float new_scale);

void menu_begin(struct menu_ctx *ctx, struct gl_win *win, float mousex, float mousey, bool clicked);
void menu_render(struct menu_ctx *ctx, struct gl_win *win);
#endif
