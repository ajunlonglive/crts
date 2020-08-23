#include "posix.h"

#include <math.h>
#include <string.h>
#include <time.h>

#include "shared/opengl/render/text.h"
#include "shared/util/log.h"
#include "terragen/opengl/render/menu.h"
#include "terragen/opengl/worker.h"

static const vec4 clrs[] = {
	{ 1, 1, 1, 0.8 },
	{ 1, 1, 1, 1 },
	{ 0, 1, 0, 1 },
};

enum element_type {
	e_none,
	e_slider,
	e_shuffle,
	e_inc,
	e_dec,
	e_button,
};

struct elem {
	int32_t opt;

	enum element_type e;

	float a, b;
	uint32_t step;

	char *label;
	union { float *f; uint32_t *u; } d;
	enum tg_dtype t;

	float x, y;
	uint32_t width;

	bool active;
};

static struct {
	float x, y;
	int32_t active;
	size_t maxl;
	struct pointf mouse;
	uint32_t mb_pressed, mb_released;
} menu = { 0, 0, -1, 0 };

static bool
ui_button_cb(void *_e, bool hvr, float x, float y, const char *str)
{
	uint8_t clri = 0;
	bool ret = false;

	if (hvr) {
		clri = 1;
		if (menu.mb_pressed & 1) {
			clri = 2;
		}

		if (menu.mb_released & 1) {
			ret = true;
		}
	}

	gl_write_string(x, y, 1.0, clrs[clri], str);

	return ret;
}

static bool
ui_shuffle(struct elem *e)
{
	assert(e->t == dt_int);

	char fmt[32] = { 0 };
	snprintf(fmt, 31, "%%%dd", e->width + 1);

	if (gl_mprintf(e->x, e->y, ta_left, &menu.mouse, e, ui_button_cb, fmt, *e->d.u)) {
		struct timespec ts;
		clock_gettime(CLOCK_MONOTONIC, &ts);
		*e->d.u = ts.tv_nsec;
		return true;
	}

	return false;
}

#define BL 64
static bool
ui_button(struct elem *e)
{
	char fmt[BL] = { 0 };

	uint32_t tw = e->width + 1;

	switch ((uint32_t)e->t) {
	case dt_int:
		snprintf(fmt, BL, "%%-%d.%ds%%%dd", tw / 2, tw / 2, tw / 2);
		return gl_mprintf(e->x, e->y, ta_left, &menu.mouse, e, ui_button_cb, fmt,
			e->label, *e->d.u);
	case dt_float:
		snprintf(fmt, BL, "%%%d.2f", tw);
		return gl_mprintf(e->x, e->y, ta_left, &menu.mouse, e, ui_button_cb, fmt,
			*e->d.f);
	case dt_none:
		snprintf(fmt, BL, "%%-%d.%ds", tw, tw);
		return gl_mprintf(e->x, e->y, ta_left, &menu.mouse, e, ui_button_cb, fmt,
			e->label);
	}

	return false;
}

static bool
slider_cb(void *_e, bool hvr, float x, float y, const char *str)
{
	struct elem *e = _e;

	uint8_t clri = 0;
	bool set = false;
	float val = 0;

	if (hvr && (menu.mb_pressed & 1 || menu.mb_released & 1)) {
		float mx = menu.mouse.x - 0.5;

		if (mx < x) {
			mx = x;
		} else if (mx > x + e->width) {
			mx = x + e->width;
		}

		val = ((mx - x) / (float)e->width) * (e->b - e->a) + e->a;

		if (e->step) {
			uint32_t rem = (uint32_t)val % e->step;
			val = floorf(val) - rem;
		} else if (e->t == dt_int) {
			val = roundf(val);
		}

		switch (e->t) {
		case dt_float:
			*e->d.f = val;
			break;
		case dt_int:
			*e->d.u = roundf(val);
			break;
		case dt_none:
			assert(false);
			break;
		}

		clri = 1;
	} else {
		val = e->t == dt_float ? *e->d.f : (float)*e->d.u;
	}

	if (hvr && menu.mb_released & 1) {
		set = true;
	}

	float pos = x + ((val - e->a) / (e->b - e->a) * e->width);

	gl_write_string(pos, y, 1.0, clrs[clri], "*");
	gl_write_string(x, y, 1.0, clrs[0], str);

	return set;
}

static bool
ui_slider(struct elem *e)
{
	gl_printf(e->x, e->y, ta_left, "%s", e->label);

	e->x += menu.maxl + 1;

	char box[e->width + 1];
	memset(box, ' ', e->width + 1);
	box[e->width + 1] = 0;

	float sx, sy;
	screen_coords_to_text_coords(e->x, e->y, &sx, &sy);

	bool ret = gl_mprintf(e->x, e->y, ta_left, &menu.mouse, e, slider_cb, box);

	e->x += e->width + 2;

	switch (e->t) {
	case dt_float:
		gl_printf(e->x, e->y, ta_left, "%0.2f", *e->d.f);
		break;
	case dt_int:
		gl_printf(e->x, e->y, ta_left, "%d", *e->d.u);
		break;
	case dt_none:
		break;
	}

	return ret;
}

static bool
ui_elem(struct elem *e)
{
	bool ret = false;

	switch (e->e) {
	case e_slider:
		ret = ui_slider(e);
		break;
	case e_shuffle:
		ret = ui_shuffle(e);
		break;
	case e_inc:
		ret = ui_shuffle(e);
		break;
	case e_dec:
		ret = ui_shuffle(e);
		break;
	case e_button:
		ret = ui_button(e);
		break;
	case e_none:
		assert(false);
		return false;
	}

	if (ret) {
		menu.active = -1;
	}

	return ret;
}

enum ui_controls {
	uic_write = tg_opt_count,
	uic_opacity,
};

struct elem elements[] = {
	{ 0 },
	{ 0 },
	{ tg_seed,           e_button, .label = "shuf seed" },
	{ tg_radius,         e_slider, 0.25, 0.5 },
	{ tg_dim,            e_slider, 128, 2048, 128 },
	{ tg_points,         e_slider, 0, 10000 },
	{ 0 },
	{ tg_mountains,      e_slider, 0, 100 },
	{ tg_valleys,        e_slider, 0, 100 },
	{ tg_fault_radius,   e_slider, 0, 100 },
	{ tg_fault_curve,    e_slider, 0, 1 },
	{ tg_height_mod,     e_slider, 0, 20 },
	{ 0 },
	{ tg_erosion_cycles, e_slider, 0, 10000 },
	{ 0 },
	{ tg_noise,          e_slider, 0, 1 },
	{ 0 },
	{ tg_upscale,        e_slider, 1, 4 },
	{ -1 }
};

float float_val = 0;
uint32_t int_val;

void
render_menu_init(struct ui_ctx *ctx)
{
	uint32_t i;

	for (i = 0; elements[i].opt >= 0; ++i) {
		if (!elements[i].e) {
			continue;
		} else if (elements[i].opt >= tg_opt_count) {
			/* elements[i].t = (enum tg_dtype)dt_none; */
			/* elements[i].width = 10; */
			continue;
		}

		if (!elements[i].label) {
			elements[i].label = terragen_opt_info[elements[i].opt].name;
		}
		elements[i].width = 20;

		size_t l;
		if ((l = strlen(elements[i].label)) > menu.maxl) {
			menu.maxl = l;
		}

		switch (elements[i].t = terragen_opt_info[elements[i].opt].t) {
		case dt_float:
			elements[i].d.f = &ctx->opts[elements[i].opt].f;
			break;
		case dt_int:
			elements[i].d.u = &ctx->opts[elements[i].opt].u;
			break;
		}
	}
}

struct elem static_elements[] = {
	/* elements[i].t = (enum tg_dtype)dt_none; */
	/* elements[i].width = 10; */
	{ uic_write, e_button, .width = 6, .x = 0, .y = 0, .label = "save", },
};

void
render_menu(struct ui_ctx *ctx)
{
	uint32_t i;
	menu.mouse = (struct pointf){
		ctx->mousex / ctx->text_scale,
		(ctx->win.height - ctx->mousey) / ctx->text_scale
	};
	menu.mb_pressed = ctx->mb_pressed;
	menu.mb_released = ctx->mb_released;

	ui_elem(&static_elements[0]);

	for (i = 0; elements[i].opt >= 0; ++i) {
		if (!elements[i].e) {
			continue;
		}

		elements[i].x = menu.x;
		elements[i].y = menu.y + i * 1.2;

		if (ui_elem(&elements[i])) {
			switch (elements[i].opt) {
			case tg_seed:
			{
				struct timespec ts;
				clock_gettime(CLOCK_MONOTONIC, &ts);
				*elements[i].d.u = ts.tv_nsec;
			}
			break;
			case tg_radius:
			case tg_dim:
				ctx->dim_changed = true;
			/* FALLTHROUGH */
			case tg_points:
			case tg_mountains:
			case tg_valleys:
			case tg_fault_radius:
			case tg_fault_curve:
			case tg_height_mod:
			case tg_erosion_cycles:
			case tg_noise:
			case tg_upscale:
				start_genworld_worker(ctx);
				break;
			case uic_write:
				ctx->write_file = true;
				break;
			default:
				break;
			}
		}
	}
}
