#include "posix.h"

#include <math.h>
#include <stdlib.h>

#include "client/input/action_handler.h"
#include "client/ui/opengl/globals.h"
#include "client/ui/opengl/hud.h"
#include "client/ui/opengl/text.h"
#include "shared/constants/globals.h"
#include "shared/util/log.h"

#define MENU_GLUE 1.5

struct menu_item {
	const char *title;
	uint32_t sub;
	long val;
	void ((*action)(struct hiface *));
};

struct menu {
	struct menu_item *items;
	uint32_t len;
};

enum submenu {
	sub_main,
	sub_harvest,
	sub_build,
	sub_carry,
};

static struct menu menu[64] = { 0 };
static struct menu_item items[64] = { 0 };

#define SENS 0.05
#define SCALE 0.9
#define MENU_R 5.0
#define SUBMENU_R 14.0
#define SPIN (PI / 4)
#define BUMP

static uint8_t
write_menu(float x, float y, float r, float sel, bool sub, int16_t selsub,
	struct menu *m, struct hiface *hf)
{
	vec4 clr = { 1, 1, 1, 0.3 };
	vec4 sel_clr = { 0, 1, 0, 0.8 };

	float cx, cy, ct, theta = 2 * PI / m->len;
	bool selected;

	uint16_t i = 0;
	int16_t sel_i = -1;

	ct = 2 * PI - theta;

	for (i = 0; i < m->len; ++i) {
		cy = ((r * sin(ct + theta / 2)) + y);
		cx = ((r * cos(ct + theta / 2)) + x);

		if (sel_i < 0) {
			if (sub) {
				selected = selsub >= 0 ? sel > ct : false;
			} else {
				selected = selsub >= 0 ? selsub == i : sel > ct;
			}
		} else {
			selected = false;
		}

		if (selected) {
			sel_i = i;

			if (m->items[i].action) {
				hf->num_override.override = true;
				hf->num_override.val = m->items[i].val;
				m->items[i].action(hf);
			}
		}

		gl_write_string_centered(cx, cy, SCALE, selected ? sel_clr : clr,
			m->items[i].title);

		if (selected && !sub && m->items[i].sub) {
			write_menu(x, y, SUBMENU_R, sel, true, selsub,
				&menu[m->items[i].sub], hf);
		}

		ct -= theta;
	}

	return sel_i;
}

static void
render_hud_menu(float x, float y, struct opengl_ui_ctx *ctx, struct hiface *hf)
{
	float dx = ctx->mouse.dx * SENS, dy = ctx->mouse.dy * SENS * -1;

	static float mx = 0, my = 0;
	static int16_t selsub = -1;

	mx += dx;
	my += dy;

	float th = atan2f(my, mx);
	float rsq = mx * mx + my * my;

	if (rsq > 1) {
		my = ((sin(th)));
		mx = ((cos(th)));
	}

	float s = atan2f(my, mx);

	if (s < 0) {
		s += 2 * PI;
	}

	uint8_t i = write_menu(x, y, MENU_R, s, false, selsub, &menu[0], hf);

	if (ctx->keyboard.mod & mod_shift && menu[0].items[i].sub) {
		selsub = i;
	} else {
		selsub = -1;
	}
}

static void
gen_menu(void)
{
	size_t i, mi = 0, len = 0;

	menu[0].items = &items[mi];
	menu[0].len = action_type_count;
	for (i = 0; i < menu[0].len; ++i) {
		switch ((enum action_type)i) {
		case at_harvest:
			menu[0].items[i].sub = sub_harvest;
			break;
		case at_build:
			menu[0].items[i].sub = sub_build;
			break;
		case at_carry:
			menu[0].items[i].sub = sub_carry;
			break;
		default:
			break;
		}

		menu[0].items[i].title = gcfg.actions[i].name;
		menu[0].items[i].action = set_action_type;
		menu[0].items[i].val = i;
		++mi;
	}

	menu[sub_harvest].items = &items[mi];
	len = 0;
	for (i = 0; i < tile_count; ++i) {
		if (!gcfg.tiles[i].hardness) {
			continue;
		}

		menu[sub_harvest].items[len].title = gcfg.tiles[i].name;
		menu[sub_harvest].items[len].action = set_action_target;
		menu[sub_harvest].items[len].val = i;

		++len;
		++mi;
	}
	menu[sub_harvest].len = len;

	menu[sub_build].items = &items[mi];
	len = 0;
	for (i = 0; i < buildings_count; i += 2) {
		menu[sub_build].items[len].title = blueprints[i].name;
		menu[sub_build].items[len].action = set_action_target;
		menu[sub_build].items[len].val = i;

		++len;
		++mi;
	}
	menu[sub_build].len = len;

	menu[sub_carry].items = &items[mi];
	len = 0;
	for (i = 0; i < ent_type_count; ++i) {
		if (!gcfg.ents[i].holdable) {
			continue;
		}

		menu[sub_carry].items[len].title = gcfg.ents[i].name;
		menu[sub_carry].items[len].action = set_action_target;
		menu[sub_carry].items[len].val = i;

		++len;
		++mi;
	}
	menu[sub_carry].len = len;
}

void
render_hud(struct opengl_ui_ctx *ctx, struct hiface *hf)
{
	const char *act_tgt_nme;
	float x, y, sx, sy;
	/*
	   x and y at in game cursor position
	   x = (float)hf->cursor.x / ctx->ref.width * ctx->width;
	   y = (float)hf->cursor.y / ctx->ref.height * ctx->height;
	 */
	x = ctx->width * 0.5;
	y = ctx->height * 0.5;
	screen_coords_to_text_coords(x, y, &sx, &sy);

	text_setup_render();

	gl_printf(0, 1, "cmd: %5.5s%5.5s | im: %s",
		hf->num.buf, hf->cmd.buf, input_mode_names[hf->im]);

	gl_printf(0, 0, "view: (%4d, %4d) | cursor: (%4d, %4d)",
		hf->view.x, hf->view.y, hf->cursor.x + hf->view.x,
		hf->cursor.y + hf->view.y);

	switch (hf->next_act.type) {
	case at_harvest:
		act_tgt_nme = gcfg.tiles[hf->next_act.tgt].name;
		break;
	case at_build:
		act_tgt_nme = blueprints[hf->next_act.tgt].name;
		break;
	default:
		act_tgt_nme = NULL;
		break;
	}

	gl_printf(0, 2, "act: 0x%x %s%c %s",
		hf->next_act.flags,
		gcfg.actions[hf->next_act.type].name,
		act_tgt_nme ? ',' : ' ',
		act_tgt_nme);

	gl_printf(0, 3, "mouse: 0x%x", ctx->mouse.buttons);

	static bool setup_menu = false;
	if (!setup_menu) {
		gen_menu();
		setup_menu = true;
	}

	if (ctx->mouse.buttons & mb_2) {
		render_hud_menu(sx, sy, ctx, hf);
	}
}
