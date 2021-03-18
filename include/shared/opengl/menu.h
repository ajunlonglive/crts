#ifndef SHARED_OPENGL_MENU_H
#define SHARED_OPENGL_MENU_H

#include "shared/math/linalg.h"
#include "shared/opengl/window.h"

enum menu_theme_elems {
	menu_theme_elem_win_bg,
	menu_theme_elem_win_bar,
	menu_theme_elem_button_bg,
	menu_theme_elem_button_bg_hovered,
	menu_theme_elem_button_bg_clicked,
	menu_theme_elem_fg,
	menu_theme_elem_count,
};

typedef vec4 menu_theme_definition[menu_theme_elem_count];

struct menu_ctx {
	menu_theme_definition theme;
	float scale;

	struct {
		float x, y, h, w, pad;
		bool initialized;
	} win;

	float x, y;

	float mousex, mousey, mousedx, mousedy;
	bool clicked, released;
};

bool menu_setup(struct menu_ctx *ctx);

bool menu_button(struct menu_ctx *ctx, const char *str);
void menu_str(struct menu_ctx *ctx, const char *str);
void menu_win(struct menu_ctx *ctx, float *x, float *y, float h, float w);

void menu_begin(struct menu_ctx *ctx, float mousex, float mousey, bool clicked);
void menu_render(struct menu_ctx *ctx, struct gl_win *win);
#endif
