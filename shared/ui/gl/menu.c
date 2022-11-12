#include "posix.h"

#include <math.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "shared/input/keyboard.h"
#include "shared/ui/gl/menu.h"
#include "shared/ui/gl/render/shapes.h"
#include "shared/ui/gl/render/text.h"
#include "shared/util/log.h"
#include "shared/util/util.h"

#define WIN_PAD 0.1

struct menu_ctx menu;

void
menu_set_scale(float new_scale)
{
	menu.new_scale = new_scale;
}

void
menu_goto_bottom_right(void)
{
	menu.x = menu.gl_win->sc_width / menu.scale;
	menu.y = menu.gl_win->sc_height / menu.scale;
}

void
menu_align(float w)
{
	if (menu.center) {
		menu.x = ((menu.gl_win->sc_width / menu.scale) - w) / 2.0f;
	}
}

uint32_t
menu_rect(struct menu_rect *rect, enum menu_theme_elems clr)
{
	return render_shapes_add_rect(rect->x, rect->y, rect->h, rect->w, menu.theme[clr]);
}

void
menu_cursor(const struct pointf *p, enum menu_theme_elems clr)
{
	const float size = 1.5f,
		    cos_45 = size * 0.7071067811865476f,
		    sin_45 = size *  0.7071067811865475f;

	render_shapes_add_tri(
		p->x, p->y,
		p->x, p->y + size,
		p->x + cos_45, p->y + sin_45,
		menu.theme[clr]);
}

static bool
is_hovered(struct menu_rect *r)
{
	if (r->x < menu.mousex && menu.mousex < r->x + r->w &&
	    r->y < menu.mousey && menu.mousey < r->y + r->h) {
		menu.something_was_hovered = true;
		return true;
	}
	return false;
}

void
menu_newline(void)
{
	menu.y += menu.linesep + WIN_PAD;

	if (menu.win) {
		menu.x = menu.win->x + WIN_PAD;
	} else {
		menu.x = 0;
	}
}

static bool
clickable_rect(enum menu_theme_elems clrs[3],
	struct menu_rect *rect, bool *hovered)
{
	*hovered = is_hovered(rect);
	uint8_t clr;

	if (*hovered && (menu.clicked || menu.held)) {
		clr = 0;
	} else if (*hovered) {
		clr = 1;
	} else {
		clr = 2;
	}

	menu_rect(rect, clrs[clr]);

	return *hovered && menu.released;
}

static bool
is_dragged(struct menu_rect *rect, bool dragged)
{
	bool hovered = is_hovered(rect);

	if (hovered && menu.clicked) {
		return true;
	} else if (!(menu.held && dragged)) {
		return false;
	} else {
		return dragged;
	}
}

bool
menu_win(struct menu_win_ctx *win_ctx)
{
	bool hovered;

	struct menu_rect bar = { win_ctx->x + 1, win_ctx->y, 1, win_ctx->w - 1 };
	enum menu_theme_elems barclr;

	{
		hovered = is_hovered(&bar);
		win_ctx->dragging = is_dragged(&bar, win_ctx->dragging);

		if (win_ctx->dragging) {
			barclr = menu_theme_elem_bar_active;
			win_ctx->x += menu.mousedx;
			win_ctx->y += menu.mousedy;

			win_ctx->x = fclamp(win_ctx->x, 0, 9999999);
			win_ctx->y = fclamp(win_ctx->y, 0, 9999999);

			bar.x = win_ctx->x + 1;
			bar.y = win_ctx->y;
		} else {
			barclr = menu_theme_elem_bar;
			win_ctx->dragging = false;
		}
	}

	menu.x = win_ctx->x;
	menu.y = win_ctx->y;

	{
		if (clickable_rect((enum menu_theme_elems[3]){
			menu_theme_elem_bar_active,
			menu_theme_elem_bar_hover,
			menu_theme_elem_bar_accent,
		}, &(struct menu_rect){ menu.x, menu.y, 1, 1 }, &hovered)) {
			win_ctx->hidden = !win_ctx->hidden;
		}

		if (win_ctx->title) {
			float x = menu.x, y = menu.y;
			render_text_add(&x, &y, menu.theme[menu_theme_elem_fg], win_ctx->hidden ? "+" : "-");
			x = menu.x + 1;
			render_text_add(&x, &y, menu.theme[menu_theme_elem_fg], win_ctx->title);
		}
	}

	win_ctx->rect_bar = menu_rect(&bar, barclr);

	++menu.y;

	if (!win_ctx->hidden) {
		assert(!menu.win);

		menu.win = win_ctx;

		struct menu_rect win_rect = { menu.x, menu.y, win_ctx->h, win_ctx->w, };
		is_hovered(&win_rect); // called only to set `something_was_hovered`
		win_ctx->rect_win = menu_rect(&win_rect, menu_theme_elem_win);

		menu.y += WIN_PAD;
		menu.x = win_ctx->x + WIN_PAD;
		return true;
	} else {
		return false;
	}
}

static void
menu_win_check_size(void)
{
	if (menu.win) {
		float t;

		if ((t = (menu.x - menu.win->x)) > menu.win->w) {
			menu.win->w = t;
		}

		if ((t = (menu.y - menu.win->y)) > menu.win->h) {
			menu.win->h = t;
		}
	}
}

void
menu_win_end(void)
{
	assert(menu.win);
	render_shapes_resize(menu.win->rect_bar, 1, menu.win->w - 1);

	if (!menu.win->hidden) {
		render_shapes_resize(menu.win->rect_win, menu.win->h, menu.win->w);
	}

	menu.win = NULL;
}

void
menu_str(const char *str)
{
	menu_align(strlen(str));
	render_text_add(&menu.x, &menu.y, menu.theme[menu_theme_elem_fg], str);
	menu_win_check_size();
}

void
menu_rect_str(struct menu_rect *rect,
	enum menu_theme_elems clr, enum menu_align align, const char *str)
{
	menu_rect(rect, clr);

	uint32_t l = strlen(str);

	switch (align) {
	case menu_align_left:
		menu.x = rect->x;
		break;
	case menu_align_right:
		if (rect->w > l) {
			menu.x = rect->x + (rect->w - l);
		} else {
			menu.x = rect->x;
		}
		break;
	}

	menu.y = rect->y + ((rect->h - 1.0f) / 2.0f);
	menu_str(str);
}

#define BUFLEN 512

void
menu_rect_printf(struct menu_rect *rect,
	enum menu_theme_elems clr, enum menu_align align, const char *fmt, ...)
{
	static char buf[BUFLEN] = { 0 };
	va_list ap;

	va_start(ap, fmt);
	vsnprintf(buf, BUFLEN - 1, fmt, ap);
	va_end(ap);

	menu_rect_str(rect, clr, align, buf);
}

void
menu_printf(const char *fmt, ...)
{
	static char buf[BUFLEN] = { 0 };
	va_list ap;

	va_start(ap, fmt);
	vsnprintf(buf, BUFLEN - 1, fmt, ap);
	va_end(ap);

	menu_str(buf);
}

#define SLIDER_MIDBAR_H 0.2
#define SLIDER_KNOB_H 1.0

bool
menu_slider(struct menu_slider_ctx *sctx, float *val)
{
	const float h = 2;
	const bool ret = menu.released && sctx->dragging;

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

	menu_align(sctx->w + 1);

	struct menu_rect bg = { menu.x, menu.y, h, sctx->w + 1 };

	bool bg_hovered = is_hovered(&bg);
	menu_rect(&bg, menu_theme_elem_bar);

	{
		struct menu_rect knob = { menu.x + (sctx->pos * sctx->w),
					  menu.y,
					  h * SLIDER_KNOB_H, SLIDER_KNOB_H };

		bool hovered = is_hovered(&knob);
		enum menu_theme_elems clr;

		if (is_dragged(&knob, sctx->dragging)) {
			sctx->dragging = true;
		} else if (!hovered && bg_hovered && menu.clicked) {
			sctx->dragging = true;
			sctx->pos = (menu.mousex - (SLIDER_KNOB_H / 2.0) - bg.x) / sctx->w;
		} else {
			sctx->dragging = false;
		}

		if (sctx->dragging) {
			clr = menu_theme_elem_bar_active;
			sctx->pos = fclamp(sctx->pos + (menu.mousedx / sctx->w), 0.0, 1.0);
			if (sctx->step > 0.0f) {
				*val = (roundf(sctx->pos * sctx->steps) * sctx->step) + sctx->min;
				float spos = ((*val - sctx->min) / (sctx->max - sctx->min));
				knob.x = menu.x + (spos * sctx->w);
			} else {
				*val = (sctx->pos * (sctx->max - sctx->min)) + sctx->min;
				knob.x = menu.x + (sctx->pos * sctx->w);
			}
		} else {
			sctx->pos = ((*val - sctx->min) / (sctx->max - sctx->min));

			if (hovered) {
				clr = menu_theme_elem_bar_hover;
			} else {
				clr = menu_theme_elem_bar_accent;
			}
		}

		menu_rect(&knob, clr);
	}

	if (sctx->label) {
		uint32_t len;
		char buf[256];
		len = snprintf(buf, 255, "%s: %0.0f%s", sctx->label, *val,
			sctx->unit ? sctx->unit : "");

		float ox = menu.x, oy = menu.y;
		menu.x += (sctx->w - len) / 2.0f;
		menu.y += (h - 1.0f) / 2.0f;
		render_text_add(&menu.x, &menu.y, menu.theme[menu_theme_elem_fg], buf);
		menu.x = ox; menu.y = oy;
	}


	menu.x += sctx->w + 1;
	menu_win_check_size();

	return ret;
}

void
menu_textbox(struct menu_textbox_ctx *tctx)
{
	float w = tctx->min_w, h = 2.0f, pad = 0.2;

	uint32_t len;
	if ((len = strlen(tctx->buf)) >= w) {
		w = len + 1;
	}

	menu_align(w);

	menu_rect(&(struct menu_rect) {
		.x = menu.x,
		.y = menu.y,
		.w = w,
		.h = h
	}, menu_theme_elem_bar_active);

	bool hovered;

	if (clickable_rect(
		(enum menu_theme_elems[3]) { menu_theme_elem_win, menu_theme_elem_win, menu_theme_elem_win  },
		&(struct menu_rect) { .x = menu.x + pad / 2.0f, .y = menu.y + pad / 2.0f, .w = w - pad, .h = h - pad },
		&hovered)) {
		if (menu.textbox != tctx) {
			menu.textbox = tctx;
			menu.textbox_cursor = menu.textbox_len = strlen(tctx->buf);
		}
	}

	menu.y += (pad + h - 1) / 2.0f;
	menu.x += ((w - pad - len) / 2.0f);
	float ox = menu.x, oy = menu.y;
	render_text_add(&menu.x, &menu.y, menu.theme[menu_theme_elem_fg], tctx->buf);

	if (menu.textbox == tctx) {
		menu_rect(&(struct menu_rect) {
			.x = ox + menu.textbox_cursor,
			.y = oy,
			.w = 0.1,
			.h = 1
		}, menu_theme_elem_bar_active);
	}
}

bool
menu_button_c(struct menu_button_ctx *bctx)
{
	const float h = 1.0 * menu.button_pad;
	bool hovered, ret;

	enum menu_theme_elems clrs[3] = {
		menu_theme_elem_bar_active,
		menu_theme_elem_bar_hover,
		bctx->clr,
	};

	if (bctx->flags & menu_button_flag_disabled) {
		clrs[0] = clrs[1] = clrs[2] = menu_theme_elem_disabled;
	}

	menu_align(bctx->w);

	ret = clickable_rect(clrs, &(struct menu_rect){ menu.x, menu.y, h, bctx->w }, &hovered);

	if (hovered && !bctx->hovered) {
		if (bctx->hover_cb) {
			bctx->hover_cb();
		}
		bctx->hovered = true;
	} else if (bctx->hovered && !hovered) {
		bctx->hovered = false;
	}

	if (bctx->str) {
		uint32_t len = strlen(bctx->str);
		float ox = menu.x, oy = menu.y;
		menu.x += (bctx->w - len) / 2.0f;
		menu.y += (h - 1.0f) / 2.0f;
		render_text_add(&menu.x, &menu.y, menu.theme[menu_theme_elem_fg], bctx->str);
		menu.x = ox + bctx->w;
		menu.y = oy;
	}

	menu_win_check_size();

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
menu_button(const char *str, enum menu_button_flags flags)
{
	const float w = strlen(str) + 2.0 * menu.button_pad;

	return menu_button_c(&(struct menu_button_ctx) { .str = str, .flags = flags, .w = w });
}

bool
menu_setup(void)
{
	if (menu.initialized) {
		return true;
	}

	const menu_theme_definition default_theme = {
		[menu_theme_elem_win]            = { 0.17, 0.20, 0.28, 0.7 },
		[menu_theme_elem_bar]            = { 0.24, 0.35, 0.30, 1.0 },
		[menu_theme_elem_bar_accent]     = { 0.34, 0.15, 0.50, 1.0 },
		[menu_theme_elem_bar_accent2]    = { 0.24, 0.15, 0.20, 1.0 },
		[menu_theme_elem_bar_hover]      = { 0.60, 0.35, 0.20, 1.0 },
		[menu_theme_elem_bar_active]     = { 0.90, 0.35, 0.00, 1.0 },
		[menu_theme_elem_disabled]       = { 0.27, 0.40, 0.48, 1.0 },
		[menu_theme_elem_fg]             = { 0.90, 0.80, 0.90, 1.0 },
	};

	menu = (struct menu_ctx) { 0 };

	memcpy(menu.theme, default_theme, sizeof(menu_theme_definition));

	menu.scale = 15.0f;
	menu.new_scale = menu.scale;
	menu.button_pad = 1.0f;
	menu.linesep = 1.0;

	if (!(render_text_setup() && render_shapes_setup())) {
		return false;
	}

	menu.initialized = true;
	return true;
}

void
menu_begin(struct gl_win *win, float mousex, float mousey, bool clicked)
{
	assert(!menu.win);

	menu.gl_win = win;

	menu.x = menu.y = 0;

	mousex /= menu.scale;
	mousey /= menu.scale;

	menu.mousedx = mousex - menu.mousex;
	menu.mousedy = mousey - menu.mousey;
	menu.mousex = mousex;
	menu.mousey = mousey;

	menu.released = (menu.clicked || menu.held) && !clicked;
	if (clicked) {
		if (menu.clicked || menu.held) {
			menu.held = true;
			menu.clicked = false;
		} else if (!menu.clicked) {
			menu.clicked = true;
			menu.held = false;
		}
	} else {
		menu.clicked = false;
		menu.held = false;
	}

	menu.something_was_hovered = false;

	render_text_clear();
	render_shapes_clear();
}

void
menu_render(struct gl_win *win)
{
	static mat4 proj;

	if (win->resized || menu.scale_changed) {
		if (menu.scale_changed) {
			/*menu.mousex =menu.mousey = 0; */
			menu.scale_changed = false;
		}

		mat4 ortho, mscale;

		vec4 scale = { menu.scale, menu.scale, 0.0, 0.0 };
		gen_scale_mat4(scale, mscale);

		gen_ortho_mat4_from_lrbt(0.0, (float)win->sc_width, (float)win->sc_height, 0.0, ortho);

		mat4_mult_mat4(ortho, mscale, proj);

		render_shapes_update_proj(proj);
		render_text_update_proj(proj);
	}

	if (menu.scale != menu.new_scale) {
		menu.scale = menu.new_scale;
		menu.scale_changed = true;
	}

	/* menu_rect(ctx, &(struct menu_rect){menu.mousex - 0.5,menu.mousey - 0.5, 1, 1 }, menu_theme_elem_bar_active); */

	render_shapes();

	render_text_commit();
	render_text();
}

static bool
edit_line(char *buf, uint32_t *bufi, uint32_t *bufl, uint32_t bufm, uint8_t k, uint8_t mod)
{
	switch (k) {
	case '\b':
		if (!*bufi) {
			return false;
		}

		memmove(&buf[*bufi - 1], &buf[*bufi], *bufl - *bufi);

		--(*bufl);
		buf[*bufl] = 0;

		--(*bufi);
		break;
	case skc_left:
		if (!*bufi) {
			return false;
		}

		--(*bufi);
		break;
	case skc_right:
		if ((*bufi) >= (*bufl)) {
			return false;
		}

		++(*bufi);
		break;
	case skc_home:
	case skc_up:
		*bufi = 0;
		break;
	case skc_end:
	case skc_down:
		*bufi = *bufl;
		break;
	case '\n':
		return true;
	default:
		if (*bufl >= bufm || k >= 128) {
			return false;
		}

		memmove(&buf[*bufi + 1], &buf[*bufi], *bufl - *bufi);

		buf[*bufi] = k;
		++(*bufi);
		++(*bufl);
	}

	return false;
}

void
menu_handle_input(uint8_t key)
{
	if (menu.textbox) {
		if (key == skc_mb1) {
			menu.textbox = NULL;
		}  else if (edit_line(menu.textbox->buf, &menu.textbox_cursor, &menu.textbox_len, MENU_TEXTBOX_BUF_LEN, key, 0)) {
			menu.textbox = NULL;
		}
	}
}
