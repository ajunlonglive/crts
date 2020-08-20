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
	struct pointf *mouse;
	uint32_t mb_pressed, mb_released;

	bool active;
};

struct elem elements[] = {
	{ tg_seed,           e_shuffle },
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

static struct {
	float x, y;
	int32_t active;
	size_t maxl;
} menu = { 0, 0, -1, 0 };

static bool
ui_button_cb(void *_e, bool hvr, float x, float y, const char *str)
{
	struct elem *e = _e;
	uint8_t clri = 0;
	bool ret = false;

	if (hvr) {
		clri = 1;
		if (e->mb_pressed & 1) {
			clri = 2;
		}

		if (e->mb_released & 1) {
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

	if (gl_mprintf(e->x, e->y, ta_left, e->mouse, e, ui_button_cb, fmt, *e->d.u)) {
		struct timespec ts;
		clock_gettime(CLOCK_MONOTONIC, &ts);
		*e->d.u = ts.tv_nsec;
		return true;
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

	if (hvr && (e->mb_pressed & 1 || e->mb_released & 1)) {
		float mx = e->mouse->x - 0.5;

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
		}

		clri = 1;
	} else {
		val = e->t == dt_float ? *e->d.f : (float)*e->d.u;
	}

	if (hvr && e->mb_released & 1) {
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
	char box[e->width + 1];
	memset(box, ' ', e->width + 1);
	box[e->width + 1] = 0;

	float sx, sy;
	screen_coords_to_text_coords(e->x, e->y, &sx, &sy);

	return gl_mprintf(e->x, e->y, ta_left, e->mouse, e, slider_cb, box);
}

static bool
ui_elem(struct elem *e)
{
	switch (e->t) {
	case dt_float:
		gl_printf(e->x, e->y, ta_left, "%s: %0.2f", e->label, *e->d.f);
		break;
	case dt_int:
		gl_printf(e->x, e->y, ta_left, "%s: %d", e->label, *e->d.u);
		break;
	}

	e->x += menu.maxl * 2;

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
	case e_none:
		assert(false);
		return false;
	}

	if (ret) {
		menu.active = -1;
	}

	return ret;
}

void
render_menu_init(struct ui_ctx *ctx)
{
	uint32_t i;

	for (i = 0; elements[i].opt >= 0; ++i) {
		if (!elements[i].e) {
			continue;
		}

		L("%d", elements[i].opt);
		elements[i].label = terragen_opt_info[elements[i].opt].name;

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

void
render_menu(struct ui_ctx *ctx)
{
	uint32_t i;
	struct pointf mouse = { ctx->mousex / ctx->text_scale,
				(ctx->win.height - ctx->mousey) / ctx->text_scale };

	for (i = 0; elements[i].opt >= 0; ++i) {
		if (!elements[i].e) {
			continue;
		}

		elements[i].mouse = &mouse;
		elements[i].mb_pressed = ctx->mb_pressed;
		elements[i].mb_released = ctx->mb_released;
		elements[i].x = menu.x;
		elements[i].y = menu.y + i * 1.2;
		elements[i].width = 20;

		if (ui_elem(&elements[i])) {
			start_genworld_worker(ctx);
		}
	}
}
