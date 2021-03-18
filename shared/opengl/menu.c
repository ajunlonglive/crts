#include "posix.h"

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "shared/opengl/menu.h"
#include "shared/opengl/render/shapes.h"
#include "shared/opengl/render/text.h"
#include "shared/util/log.h"

static bool
is_hovered(struct menu_ctx *ctx, float h, float w)
{
	return ctx->x < ctx->mousex && ctx->mousex < ctx->x + w &&
	       ctx->y < ctx->mousey && ctx->mousey < ctx->y + h;
}

void
menu_win(struct menu_ctx *ctx, float *x, float *y, float h, float w)
{
	ctx->x = *x;
	ctx->y = *y;

	const bool hovered = is_hovered(ctx, 1, w),
		   clicked = hovered && ctx->clicked;
	vec4 *bg;

	if (clicked) {
		bg = &ctx->theme[menu_theme_elem_button_bg_clicked];
		*x += ctx->mousedx;
		*y += ctx->mousedy;
	} else if (hovered) {
		bg = &ctx->theme[menu_theme_elem_button_bg_hovered];
	} else {
		bg = &ctx->theme[menu_theme_elem_button_bg];
	}

	render_shapes_add_rect(*x, *y, 1, w, *bg);

	render_shapes_add_rect(*x, *y + 1, h - 1, w, ctx->theme[menu_theme_elem_win_bg]);

	ctx->win.x = *x;
	ctx->win.y = *y + 1;
	ctx->win.h = h;
	ctx->win.w = w;

	ctx->x = *x;
	ctx->y = *y + 1 + ctx->win.pad;
}

void
menu_str(struct menu_ctx *ctx, const char *str)
{
	render_text_add(&ctx->x, &ctx->y, ctx->theme[menu_theme_elem_fg], str);

	ctx->y += 1.0;
	ctx->x = ctx->win.x + ctx->win.pad;
}

void
menu_slider(struct menu_ctx *ctx, float *val, float min, float max, float step)
{
}

bool
menu_button(struct menu_ctx *ctx, const char *str)
{
	const float w = strlen(str), h = 1.0;
	const bool hovered = is_hovered(ctx, h, w),
		   clicked = hovered && ctx->clicked;
	vec4 *bg;

	if (clicked) {
		bg = &ctx->theme[menu_theme_elem_button_bg_clicked];
	} else if (hovered) {
		bg = &ctx->theme[menu_theme_elem_button_bg_hovered];
	} else {
		bg = &ctx->theme[menu_theme_elem_button_bg];
	}

	render_shapes_add_rect(ctx->x, ctx->y, h, w, *bg);

	render_text_add(&ctx->x, &ctx->y, ctx->theme[menu_theme_elem_fg], str);

	ctx->y += 1.0;
	ctx->x = ctx->win.x + ctx->win.pad;

	return hovered && ctx->released;
}

bool
menu_setup(struct menu_ctx *ctx)
{
	const menu_theme_definition default_theme = {
		[menu_theme_elem_win_bg]            = { 0, 0, 0, 1 },
		[menu_theme_elem_win_bar]           = { 1, 1, 1, 1 },
		[menu_theme_elem_button_bg]         = { 1, 0, 1, 1 },
		[menu_theme_elem_button_bg_hovered] = { 1, 1, 0, 1 },
		[menu_theme_elem_button_bg_clicked] = { 0, 1, 1, 1 },
		[menu_theme_elem_fg]                = { 1, 1, 1, 1 },

	};

	*ctx = (struct menu_ctx) { 0 };

	memcpy(ctx->theme, default_theme, sizeof(menu_theme_definition));

	ctx->scale = 15.0f;

	return render_text_setup() && render_shapes_setup();
}

void
menu_begin(struct menu_ctx *ctx, float mousex, float mousey, bool clicked)
{
	ctx->x = ctx->y = 0;

	mousex /= ctx->scale;
	mousey /= ctx->scale;

	ctx->mousedx = mousex - ctx->mousex;
	ctx->mousedy = mousey - ctx->mousey;
	ctx->mousex = mousex;
	ctx->mousey = mousey;

	ctx->released = ctx->clicked && !clicked;
	ctx->clicked = clicked;

	render_text_clear();
	render_shapes_clear();
}

void
menu_render(struct menu_ctx *ctx, struct gl_win *win)
{
	static mat4 proj;

	if (win->resized) {
		mat4 ortho, mscale;

		vec4 scale = { ctx->scale, ctx->scale, 0.0, 0.0 };
		gen_scale_mat4(scale, mscale);

		gen_ortho_mat4_from_lrbt(0.0, (float)win->width, (float)win->height, 0.0, ortho);

		mat4_mult_mat4(ortho, mscale, proj);
	}

	render_shapes(win, proj);

	render_text_commit();
	render_text(win, proj);
}
