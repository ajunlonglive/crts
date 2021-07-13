#include "posix.h"

#include <math.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "shared/ui/gl/menu.h"
#include "shared/ui/gl/render/shapes.h"
#include "shared/ui/gl/render/text.h"
#include "shared/util/log.h"
#include "shared/util/util.h"

#define WIN_PAD 0.1

void
menu_set_scale(struct menu_ctx *ctx, float new_scale)
{
	ctx->new_scale = new_scale;
}

void
menu_goto_bottom_right(struct menu_ctx *ctx)
{
	ctx->x = ctx->gl_win->sc_width / ctx->scale;
	ctx->y = ctx->gl_win->sc_height / ctx->scale;
}

void
menu_align(struct menu_ctx *ctx, float w)
{
	if (ctx->center) {
		ctx->x += ((ctx->gl_win->sc_width / ctx->scale) - w) / 2.0f;
	}
}

uint32_t
menu_rect(struct menu_ctx *ctx, struct menu_rect *rect, enum menu_theme_elems clr)
{
	return render_shapes_add_rect(rect->x, rect->y, rect->h, rect->w, ctx->theme[clr]);
}

static bool
is_hovered(struct menu_ctx *ctx, struct menu_rect *r)
{
	return r->x < ctx->mousex && ctx->mousex < r->x + r->w &&
	       r->y < ctx->mousey && ctx->mousey < r->y + r->h;
}

void
menu_newline(struct menu_ctx *ctx)
{
	ctx->y += 1 + WIN_PAD;

	if (ctx->win) {
		ctx->x = ctx->win->x + WIN_PAD;
	} else {
		ctx->x = 0;
	}
}

static bool
clickable_rect(struct menu_ctx *ctx, enum menu_theme_elems clrs[3],
	struct menu_rect *rect, bool *hovered)
{
	*hovered = is_hovered(ctx, rect);
	uint8_t clr;

	if (*hovered && (ctx->clicked || ctx->held)) {
		clr = 0;
	} else if (*hovered) {
		clr = 1;
	} else {
		clr = 2;
	}

	menu_rect(ctx, rect, clrs[clr]);

	return *hovered && ctx->released;
}

static bool
is_dragged(struct menu_ctx *ctx, struct menu_rect *rect, bool dragged)
{
	bool hovered = is_hovered(ctx, rect);

	if (hovered && ctx->clicked) {
		return true;
	} else if (!(ctx->held && dragged)) {
		return false;
	} else {
		return dragged;
	}
}

bool
menu_win(struct menu_ctx *ctx, struct menu_win_ctx *win_ctx)
{
	bool hovered;

	struct menu_rect bar = { win_ctx->x + 1, win_ctx->y, 1, win_ctx->w - 1 };
	enum menu_theme_elems barclr;

	{
		hovered = is_hovered(ctx, &bar);
		win_ctx->dragging = is_dragged(ctx, &bar, win_ctx->dragging);

		if (win_ctx->dragging) {
			barclr = menu_theme_elem_bar_active;
			win_ctx->x += ctx->mousedx;
			win_ctx->y += ctx->mousedy;

			win_ctx->x = fclamp(win_ctx->x, 0, 9999999);
			win_ctx->y = fclamp(win_ctx->y, 0, 9999999);

			bar.x = win_ctx->x + 1;
			bar.y = win_ctx->y;
		} else {
			barclr = menu_theme_elem_bar;
			win_ctx->dragging = false;
		}
	}

	ctx->x = win_ctx->x;
	ctx->y = win_ctx->y;

	{
		if (clickable_rect(ctx, (enum menu_theme_elems[3]){
			menu_theme_elem_bar_active,
			menu_theme_elem_bar_hover,
			menu_theme_elem_bar_accent,
		}, &(struct menu_rect){ ctx->x, ctx->y, 1, 1 }, &hovered)) {
			win_ctx->hidden = !win_ctx->hidden;
		}

		if (win_ctx->title) {
			float x = ctx->x, y = ctx->y;
			render_text_add(&x, &y, ctx->theme[menu_theme_elem_fg], win_ctx->hidden ? "+" : "-");
			x = ctx->x + 1;
			render_text_add(&x, &y, ctx->theme[menu_theme_elem_fg], win_ctx->title);
		}
	}

	win_ctx->rect_bar = menu_rect(ctx, &bar, barclr);

	++ctx->y;

	if (!win_ctx->hidden) {
		assert(!ctx->win);

		ctx->win = win_ctx;

		win_ctx->rect_win = render_shapes_add_rect(ctx->x, ctx->y,
			win_ctx->h, win_ctx->w,
			ctx->theme[menu_theme_elem_win]);

		ctx->y += WIN_PAD;
		ctx->x = win_ctx->x + WIN_PAD;
		return true;
	} else {
		return false;
	}
}

static void
menu_win_check_size(struct menu_ctx *ctx)
{
	if (ctx->win) {
		float t;

		if ((t = (ctx->x - ctx->win->x)) > ctx->win->w) {
			ctx->win->w = t;
		}

		if ((t = (ctx->y - ctx->win->y)) > ctx->win->h) {
			ctx->win->h = t;
		}
	}
}

void
menu_win_end(struct menu_ctx *ctx)
{
	assert(ctx->win);
	render_shapes_resize(ctx->win->rect_bar, 1, ctx->win->w - 1);

	if (!ctx->win->hidden) {
		render_shapes_resize(ctx->win->rect_win, ctx->win->h, ctx->win->w);
	}

	ctx->win = NULL;
}

void
menu_str(struct menu_ctx *ctx, const char *str)
{
	menu_align(ctx, strlen(str));
	render_text_add(&ctx->x, &ctx->y, ctx->theme[menu_theme_elem_fg], str);
	menu_win_check_size(ctx);
}

void
menu_rect_str(struct menu_ctx *ctx, struct menu_rect *rect,
	enum menu_theme_elems clr, enum menu_align align, const char *str)
{
	menu_rect(ctx, rect, clr);

	uint32_t l = strlen(str);

	switch (align) {
	case menu_align_left:
		ctx->x = rect->x;
		break;
	case menu_align_right:
		if (rect->w > l) {
			ctx->x = rect->x + (rect->w - l);
		} else {
			ctx->x = rect->x;
		}
		break;
	}

	ctx->y = rect->y + ((rect->h - 1.0f) / 2.0f);
	menu_str(ctx, str);
}

#define BUFLEN 512

void
menu_rect_printf(struct menu_ctx *ctx, struct menu_rect *rect,
	enum menu_theme_elems clr, enum menu_align align, const char *fmt, ...)
{
	static char buf[BUFLEN] = { 0 };
	va_list ap;

	va_start(ap, fmt);
	vsnprintf(buf, BUFLEN - 1, fmt, ap);
	va_end(ap);

	menu_rect_str(ctx, rect, clr, align, buf);
}

void
menu_printf(struct menu_ctx *ctx, const char *fmt, ...)
{
	static char buf[BUFLEN] = { 0 };
	va_list ap;

	va_start(ap, fmt);
	vsnprintf(buf, BUFLEN - 1, fmt, ap);
	va_end(ap);

	menu_str(ctx, buf);
}

#define SLIDER_MIDBAR_H 0.2
#define SLIDER_KNOB_H 0.8

bool
menu_slider(struct menu_ctx *ctx, struct menu_slider_ctx *sctx, float *val)
{
	const float slider_knob_margin = (1.0f - SLIDER_KNOB_H) / 2.0f;
	const bool ret = ctx->released && sctx->dragging;

	if (!sctx->init) {
		assert(sctx->min < sctx->max);
		assert(sctx->step < (sctx->max - sctx->min));

		if (*val < sctx->min) {
			sctx->pos = 0.0f;
		} else if (*val > sctx->max) {
			sctx->pos = 1.0f;
		} else {
			sctx->pos = ((*val - sctx->min) / sctx->max);
		}

		sctx->steps = (sctx->max - sctx->min) / sctx->step;

		if (sctx->w <= 0.0f) {
			sctx->w = 10.0f;
		}

		sctx->init = true;
	}

	render_shapes_add_rect(ctx->x, ctx->y, 1, sctx->w + 1, ctx->theme[menu_theme_elem_bar]);
	render_shapes_add_rect(
		ctx->x + 0.5f,
		ctx->y + ((1.0f - SLIDER_MIDBAR_H) / 2.0f),
		SLIDER_MIDBAR_H,
		sctx->w,
		ctx->theme[menu_theme_elem_bar_accent2]
		);

	{
		struct menu_rect knob = { ctx->x + slider_knob_margin + (sctx->pos * sctx->w),
					  ctx->y + slider_knob_margin,
					  SLIDER_KNOB_H, SLIDER_KNOB_H };

		bool hovered = is_hovered(ctx, &knob);
		sctx->dragging = is_dragged(ctx, &knob, sctx->dragging);
		enum menu_theme_elems clr;

		if (sctx->dragging) {
			clr = menu_theme_elem_bar_active;
			sctx->pos = fclamp(sctx->pos + (ctx->mousedx / sctx->w), 0.0, 1.0);
			if (sctx->step > 0.0f) {
				*val = (ceilf(sctx->pos * sctx->steps) * sctx->step) + sctx->min;
			} else {
				*val = (sctx->pos * (sctx->max - sctx->min)) + sctx->min;
			}
			knob.x = ctx->x + slider_knob_margin + (sctx->pos * sctx->w);
		} else {
			sctx->pos = ((*val - sctx->min) / sctx->max);

			if (hovered) {
				clr = menu_theme_elem_bar_hover;
			} else {
				clr = menu_theme_elem_bar_accent;
			}
		}

		menu_rect(ctx, &knob, clr);
	}

	ctx->x += sctx->w + 1;
	menu_win_check_size(ctx);

	return ret;
}

bool
menu_button_c(struct menu_ctx *ctx, struct menu_button_ctx *bctx)
{
	uint32_t len = strlen(bctx->str);
	const float h = 1.0 * ctx->button_pad;
	bool hovered, ret;

	enum menu_theme_elems clrs[3] = {
		menu_theme_elem_bar_active,
		menu_theme_elem_bar_hover,
		menu_theme_elem_bar,
	};

	if (bctx->flags & menu_button_flag_disabled) {
		clrs[0] = clrs[1] = clrs[2] = menu_theme_elem_disabled;
	}

	menu_align(ctx, bctx->w);

	ret = clickable_rect(ctx, clrs, &(struct menu_rect){ ctx->x, ctx->y, h, bctx->w }, &hovered);

	if (hovered && !bctx->hovered) {
		if (bctx->hover_cb) {
			bctx->hover_cb();
		}
		bctx->hovered = true;
	} else if (bctx->hovered && !hovered) {
		bctx->hovered = false;
	}

	float ox = ctx->x, oy = ctx->y;
	ctx->x += (bctx->w - len) / 2.0f;
	ctx->y += (h - 1.0f) / 2.0f;
	render_text_add(&ctx->x, &ctx->y, ctx->theme[menu_theme_elem_fg], bctx->str);
	ctx->x = ox + bctx->w;

	ctx->y = oy;

	menu_win_check_size(ctx);

	if (bctx->flags & menu_button_flag_disabled) {
		return false;
	} else {
		if (ret && bctx->click_cb) {
			bctx->click_cb();
		}
		return ret;
	}
}

bool
menu_button(struct menu_ctx *ctx, const char *str, enum menu_button_flags flags)
{
	const float w = strlen(str) + 2.0 * ctx->button_pad;

	return menu_button_c(ctx, &(struct menu_button_ctx) { .str = str, .flags = flags, .w = w });
}

bool
menu_setup(struct menu_ctx *ctx)
{
	const menu_theme_definition default_theme = {
		[menu_theme_elem_win]            = { 0.17, 0.20, 0.28, 0.7 },
		[menu_theme_elem_bar]            = { 0.24, 0.35, 0.30, 1.0 },
		[menu_theme_elem_bar_accent]     = { 0.34, 0.15, 0.50, 1.0 },
		[menu_theme_elem_bar_accent2]    = { 0.54, 0.35, 0.80, 1.0 },
		[menu_theme_elem_bar_hover]      = { 0.60, 0.35, 0.20, 1.0 },
		[menu_theme_elem_bar_active]     = { 0.90, 0.35, 0.00, 1.0 },
		[menu_theme_elem_disabled]       = { 0.27, 0.40, 0.48, 1.0 },
		[menu_theme_elem_fg]             = { 0.90, 0.80, 0.90, 1.0 },
	};

	*ctx = (struct menu_ctx) { 0 };

	memcpy(ctx->theme, default_theme, sizeof(menu_theme_definition));

	ctx->scale = 15.0f;
	ctx->new_scale = ctx->scale;

	return render_text_setup() && render_shapes_setup();
}

void
menu_begin(struct menu_ctx *ctx, struct gl_win *win, float mousex, float mousey, bool clicked)
{
	assert(!ctx->win);

	ctx->gl_win = win;

	ctx->x = ctx->y = 0;

	mousex /= ctx->scale;
	mousey /= ctx->scale;

	ctx->mousedx = mousex - ctx->mousex;
	ctx->mousedy = mousey - ctx->mousey;
	ctx->mousex = mousex;
	ctx->mousey = mousey;

	ctx->released = (ctx->clicked || ctx->held) && !clicked;
	if (clicked) {
		if (ctx->clicked || ctx->held) {
			ctx->held = true;
			ctx->clicked = false;
		} else if (!ctx->clicked) {
			ctx->clicked = true;
			ctx->held = false;
		}
	} else {
		ctx->clicked = false;
		ctx->held = false;
	}

	render_text_clear();
	render_shapes_clear();
}

void
menu_render(struct menu_ctx *ctx, struct gl_win *win)
{
	static mat4 proj;

	if (win->resized || ctx->scale_changed) {
		if (ctx->scale_changed) {
			/* ctx->mousex = ctx->mousey = 0; */
			ctx->scale_changed = false;
		}

		mat4 ortho, mscale;

		vec4 scale = { ctx->scale, ctx->scale, 0.0, 0.0 };
		gen_scale_mat4(scale, mscale);

		gen_ortho_mat4_from_lrbt(0.0, (float)win->sc_width, (float)win->sc_height, 0.0, ortho);

		mat4_mult_mat4(ortho, mscale, proj);

		render_shapes_update_proj(proj);
		render_text_update_proj(proj);
	}

	if (ctx->scale != ctx->new_scale) {
		ctx->scale = ctx->new_scale;
		ctx->scale_changed = true;
	}

	/* menu_rect(ctx, &(struct menu_rect){ ctx->mousex - 0.5, ctx->mousey - 0.5, 1, 1 }, menu_theme_elem_bar_active); */

	render_shapes();

	render_text_commit();
	render_text();
}
