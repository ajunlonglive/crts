#include "posix.h"

#include <string.h>

#include "client/ui/opengl/input_types.h"
#include "client/ui/opengl/keymap_hook.h"
#include "client/ui/opengl/render.h"
#include "client/ui/opengl/ui.h"
#include "shared/util/inih.h"
#include "shared/util/log.h"
#include "shared/util/text.h"

enum tables {
	table_sections,
	table_mtt,
	table_mods,
	table_mouse,
	table_keys,
	table_key_commands,
};

enum opengl_keymap_section {
	opengl_mouse,
	opengl_keyboard,
	opengl_mouse_flying,
	opengl_keyboard_flying,
	opengl_mouse_released,
};

const enum opengl_input_mode section_to_input_mode[] = {
	[opengl_mouse] = oim_normal,
	[opengl_keyboard] = oim_normal,
	[opengl_mouse_flying] = oim_flying,
	[opengl_keyboard_flying] = oim_flying,
	[opengl_mouse_released] = oim_released,
};

static struct cfg_lookup_table ltbl[] = {
	[table_sections] = {
		"opengl/mouse",           opengl_mouse,
		"opengl/keyboard",        opengl_keyboard,
		"opengl/mouse/flying",    opengl_mouse_flying,
		"opengl/keyboard/flying", opengl_keyboard_flying,
		"opengl/mouse/released",  opengl_mouse_released,
	},
	[table_mtt] = {
		"click",  mmt_click,
		"drag",   mmt_drag,
		"scroll", mmt_scroll,
	},
	[table_mods] = {
		"shift", mod_shift,
		"ctrl", mod_ctrl,
	},
	[table_mouse] = {
		"mb1", mb_1,
		"mb2", mb_2,
		"mb3", mb_3,
		"mb4", mb_4,
		"mb5", mb_5,
		"mb6", mb_6,
		"mb7", mb_7,
		"mb8", mb_8,
	},
	[table_keys] = {
		/* from https://www.glfw.org/docs/latest/group__keys.html */
		"space", 32, "apostrophe", 39, "comma", 44, "minus", 45,
		"period", 46, "slash", 47, "semicolon", 59, "equal", 61, "0",
		48, "1", 49, "2", 50, "3", 51, "4", 52, "5", 53, "6", 54, "7",
		55, "8", 56, "9", 57, "a", 65, "b", 66, "c", 67, "d", 68, "e",
		69, "f", 70, "g", 71, "h", 72, "i", 73, "j", 74, "k", 75, "l",
		76, "m", 77, "n", 78, "o", 79, "p", 80, "q", 81, "r", 82, "s",
		83, "t", 84, "u", 85, "v", 86, "w", 87, "x", 88, "y", 89, "z",
		90, "left_bracket", 91, "backslash", 92, "right_bracket", 93,
		"grave_accent", 96, "world_1", 161, "world_2", 162, "escape",
		256, "enter", 257, "tab", 258, "backspace", 259, "insert", 260,
		"delete", 261, "right", 262, "left", 263, "down", 264, "up",
		265, "page_up", 266, "page_down", 267, "home", 268, "end", 269,
		"caps_lock", 280, "scroll_lock", 281, "num_lock", 282,
		"print_screen", 283, "pause", 284, "f1", 290, "f2", 291, "f3",
		292, "f4", 293, "f5", 294, "f6", 295, "f7", 296, "f8", 297,
		"f9", 298, "f10", 299, "f11", 300, "f12", 301, "f13", 302,
		"f14", 303, "f15", 304, "f16", 305, "f17", 306, "f18", 307,
		"f19", 308, "f20", 309, "f21", 310, "f22", 311, "f23", 312,
		"f24", 313, "f25", 314, "kp_0", 320, "kp_1", 321, "kp_2", 322,
		"kp_3", 323, "kp_4", 324, "kp_5", 325, "kp_6", 326, "kp_7",
		327, "kp_8", 328, "kp_9", 329, "kp_decimal", 330, "kp_divide",
		331, "kp_multiply", 332, "kp_subtract", 333, "kp_add", 334,
		"kp_enter", 335, "kp_equal", 336, "left_shift", 340,
		"left_control", 341, "left_alt", 342, "left_super", 343,
		"right_shift", 344, "right_control", 345, "right_alt", 346,
		"right_super", 347, "menu", 348,
	},
	[table_key_commands] = {
		"toggle_render_step_ents",        okc_toggle_render_step_ents,
		"toggle_render_step_selection",   okc_toggle_render_step_selection,
		"toggle_render_step_chunks",      okc_toggle_render_step_chunks,
		"toggle_render_step_shadows",     okc_toggle_render_step_shadows,
		"toggle_render_step_reflections", okc_toggle_render_step_reflections,
		"toggle_wireframe",       okc_toggle_wireframe,
		"toggle_camera_lock",     okc_toggle_camera_lock,
		"toggle_debug_hud",       okc_toggle_debug_hud,
		"toggle_look_angle",      okc_toggle_look_angle,
		"release_mouse",          okc_release_mouse,
		"capture_mouse",          okc_capture_mouse,
		"fly_forward",            okc_fly_forward,
		"fly_left",               okc_fly_left,
		"fly_right",              okc_fly_right,
		"fly_back",               okc_fly_back,
	},
};

static struct cfg_lookup_table mouse_action_ltbl[] = {
	[mmt_drag] = {
		"noop",                mad_noop,
		"move_view",           mad_move_view,
		"move_cursor_neutral", mad_move_cursor_neutral,
		"move_cursor_create",  mad_move_cursor_create,
		"move_cursor_destroy", mad_move_cursor_destroy,
		"point_camera",        mad_point_camera,
	},
	[mmt_scroll] = {
		"noop",                mas_noop,
		"zoom",                mas_zoom,
		"quick_zoom",          mas_quick_zoom,
	},
};

static uint32_t
strlen_till(const char *s, char c)
{
	uint32_t l;
	for (l = 0; s[l]; ++l) {
		if (s[l] == c) {
			break;
		}
	}

	return l;
}

static bool
parse_mouse_map(char *err, struct opengl_ui_ctx *ctx, const char *k, const char *v,
	enum opengl_input_mode oim)
{
	uint32_t l;
	int32_t i, key;
	const char *type = k, *sep, *trig = NULL;
	struct opengl_mouse_map mm = { 0 };

	sep = strchr(k, ' ');
	l = sep ? sep - k : (uint32_t)strlen(k);

	if ((i = cfg_string_lookup_n(type, &ltbl[table_mtt], l)) == -1) {
		INIH_ERR("invalid mouse trigger type");
		return false;
	}

	mm.type = i;

	if (!sep) {
		goto parse_mouse_cmd;
	}

	while (*(sep + 1) && is_whitespace(*(sep + 1))) {
		++sep;
	}

	do {
		if (!*(trig = sep + 1)) {
			break;
		}

		if (is_whitespace(*trig)) {
			++trig;
			continue;
		}

		l = strlen_till(trig, '+');

		if ((key = cfg_string_lookup_n(trig, &ltbl[table_mods], l)) != -1) {
			mm.mod |= key;
		} else if ((key = cfg_string_lookup_n(trig, &ltbl[table_mouse], l)) != -1) {
			mm.button |= key;
		} else {
			INIH_ERR("invalid mouse button / modifier: '%s'", trig);
			return false;
		}
	} while ((sep = strchr(trig, '+')));

parse_mouse_cmd:

	if (mm.type == mmt_click) {
		if (!mm.button) {
			INIH_ERR("a click map must have at least one button");
			return false;
		}

		mm.action.click.is_opengl_kc = true;
		if ((i = cfg_string_lookup(v, &ltbl[table_key_commands])) == -1) {
			mm.action.click.is_opengl_kc = false;

			i = cfg_string_lookup(v,
				&cmd_string_lookup_tables[cslt_commands]);

			assert(i != kc_macro); // TODO: can this happen?
		}
	} else {
		i = cfg_string_lookup(v, &mouse_action_ltbl[mm.type]);
	}

	if (i == -1) {
		INIH_ERR("invalid mouse action '%s' for map type", v);
		return false;
	}

	switch (mm.type) {
	case mmt_click:
		mm.action.click.kc = i;
		break;
	case mmt_drag:
		mm.action.drag = i;
		break;
	case mmt_scroll:
		mm.action.scroll = i;
		break;
	}

	if (ctx->input_maps[oim].mouse_len >= MAX_OPENGL_MAPS) {
		INIH_ERR("too many mouse maps");
		return false;
	}

	ctx->input_maps[oim].mouse[ctx->input_maps[oim].mouse_len++] = mm;

	return true;
}

static bool
parse_key_map(char *err, struct opengl_ui_ctx *ctx, const char *k, const char *v,
	enum opengl_input_mode oim)
{
	uint32_t l;
	int32_t i, key;
	const char *sep = k - 1, *trig = NULL;
	struct opengl_key_map km = { 0 };
	bool key_set = false;

	do {
		if (!sep || !*(trig = sep + 1)) {
			break;
		}

		if (is_whitespace(*trig)) {
			++trig;
			continue;
		}

		l = strlen_till(trig, '+');

		if ((key = cfg_string_lookup_n(trig, &ltbl[table_mods], l)) != -1) {
			km.mod |= key;
		} else if ((key = cfg_string_lookup_n(trig, &ltbl[table_keys], l)) != -1) {
			if (key_set) {
				INIH_ERR("more than one key in mapping");
				return false;
			}

			km.key = key;
			key_set = true;
		} else {
			INIH_ERR("invalid key / modifier: '%s'", trig);
			return false;
		}
	} while ((sep = strchr(trig, '+')));

	if (!key_set) {
		INIH_ERR("a mapping must have a key");
		return false;
	}

	if ((i = cfg_string_lookup(v, &ltbl[table_key_commands])) == -1) {
		INIH_ERR("invalid key action '%s'", v);
		return false;
	}

	km.action = i;

	if (ctx->input_maps[oim].mouse_len >= MAX_OPENGL_MAPS) {
		INIH_ERR("too many key maps");
		return false;
	}

	ctx->input_maps[oim].keyboard[ctx->input_maps[oim].keyboard_len++] = km;

	return true;
}

enum keymap_hook_result
opengl_ui_keymap_hook(struct opengl_ui_ctx *ctx, char *err, const char *sec, const char *k,
	const char *v, uint32_t line)
{
	int32_t i;

	if ((i = cfg_string_lookup(sec, &ltbl[table_sections])) == -1) {
		return khr_unmatched;
	}

	enum opengl_keymap_section section = i;

	switch (section) {
	case opengl_mouse:
	case opengl_mouse_flying:
	case opengl_mouse_released:
		if (!parse_mouse_map(err, ctx, k, v, section_to_input_mode[section])) {
			return khr_failed;
		}

		break;
	case opengl_keyboard:
	case opengl_keyboard_flying:
		if (!parse_key_map(err, ctx, k, v, section_to_input_mode[section])) {
			return khr_failed;
		}

		break;
	}

	return khr_matched;
}
