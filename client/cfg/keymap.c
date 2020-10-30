#include "posix.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "client/cfg/keymap.h"
#include "client/input/handler.h"
#include "client/input/keymap.h"
#include "shared/sim/action.h"
#include "shared/sim/chunk.h"
#include "shared/util/inih.h"
#include "shared/util/log.h"

enum tables {
	table_im,
	table_constants
};

const struct cfg_lookup_table key_command_ltbl = {
	"none", kc_none,
	"invalid", kc_invalid,
	"center", kc_center,
	"center_cursor", kc_center_cursor,
	"view_left", kc_view_left,
	"view_down", kc_view_down,
	"view_up", kc_view_up,
	"view_right", kc_view_right,
	"find", kc_find,
	"set_input_mode", kc_set_input_mode,
	"quit", kc_quit,
	"cursor_left", kc_cursor_left,
	"cursor_down", kc_cursor_down,
	"cursor_up", kc_cursor_up,
	"cursor_right", kc_cursor_right,
	"set_action_type", kc_set_action_type,
	"set_action_target", kc_set_action_target,
	"toggle_action_flag", kc_toggle_action_flag,
	"read_action_target", kc_read_action_target,
	"swap_cursor_with_source", kc_swap_cursor_with_source,
	"set_action_height", kc_set_action_height,
	"action_height_grow", kc_action_height_grow,
	"action_height_shrink", kc_action_height_shrink,
	"set_action_width", kc_set_action_width,
	"action_width_grow", kc_action_width_grow,
	"action_width_shrink", kc_action_width_shrink,
	"action_rect_rotate", kc_action_rect_rotate,
	"undo_action", kc_undo_action,
	"exec_action", kc_exec_action,
	"toggle_help", kc_toggle_help,
	"debug_pathfind_toggle", kc_debug_pathfind_toggle,
	"debug_pathfind_place_point", kc_debug_pathfind_place_point,
	"", kc_macro,
};

static struct cfg_lookup_table ltbl[] = {
	[table_im] = {
		"normal", im_normal,
		"select", im_select,
		"resize", im_resize,
		"cmd", im_cmd,
	},
	[table_constants] = {
		"tile_deep_water", tile_deep_water,
		"tile_water", tile_water,
		"tile_wetland", tile_wetland,
		"tile_plain", tile_plain,
		"tile_forest", tile_forest,
		"tile_mountain", tile_mountain,
		"tile_peak", tile_peak,
		"tile_dirt", tile_dirt,
		"tile_forest_young", tile_forest_young,
		"tile_forest_old", tile_forest_old,
		"tile_wetland_forest_young", tile_wetland_forest_young,
		"tile_wetland_forest", tile_wetland_forest,
		"tile_wetland_forest_old", tile_wetland_forest_old,
		"tile_coral", tile_coral,
		"tile_stream", tile_stream,
		"tile_wood", tile_wood,
		"tile_stone", tile_stone,
		"tile_wood_floor", tile_wood_floor,
		"tile_rock_floor", tile_rock_floor,
		"tile_storehouse", tile_storehouse,
		"tile_farmland_empty", tile_farmland_empty,
		"tile_farmland_done", tile_farmland_done,
		"tile_burning", tile_burning,
		"tile_burnt", tile_burnt,
		"at_none", at_none,
		"at_move", at_move,
		"at_harvest", at_harvest,
		"at_build", at_build,
		"at_fight", at_fight,
		"at_carry", at_carry,
		"im_select", im_select,
		"im_normal", im_normal,
		"im_resize", im_resize,
		"im_cmd", im_cmd,
	},
};

uint8_t
parse_cfg_keymap_key(const char *str, uint8_t *consumed)
{
	if (!str[0]) {
		*consumed = 0;
		return 0;
	}

	switch (str[0]) {
	case '\\':
		if (!str[1]) {
			*consumed = 0;
			return 0;
		}

		*consumed = 2;

		switch (str[1]) {
		case 'u':
			return skc_up;
		case 'd':
			return skc_down;
		case 'l':
			return skc_left;
		case 'r':
			return skc_right;
		case 'n':
			return '\n';
		case 't':
			return '\t';
		case 's':
			return ' ';
		default:
			return str[1];
		}
	default:
		*consumed = 1;
		return str[0];
	}
}
#define CONST_BUF_LEN 32

static bool
parse_macro(char *err, char *buf, const char *macro)
{
	uint32_t i, bufi = 0, const_bufi, mode = 0;
	int32_t constant;
	char const_buf[CONST_BUF_LEN + 1] = { 0 };

	for (i = 0; macro[i] != '\0'; ++i) {
		if (macro[i] == '\0') {
			break;
		} else if (mode == 1) {
			if (macro[i] == '>') {
				const_buf[const_bufi] = 0;

				if ((constant = cfg_string_lookup(const_buf,
					&ltbl[table_constants])) == -1) {
					INIH_ERR("invalid constant name: '%s' while parsing macro: '%s'",
						const_buf, macro);
					return false;
				}

				bufi += snprintf(&buf[bufi], KEYMAP_MACRO_LEN - bufi, "%d", constant);

				mode = 0;
			} else {
				const_buf[const_bufi++] = macro[i];

				if (const_bufi >= CONST_BUF_LEN) {
					INIH_ERR("const too long while parsing macro: '%s'", macro);
					return false;
				}
			}

		} else if (macro[i] == '<') {
			const_bufi = 0;
			mode = 1;
		} else {
			buf[bufi++] = macro[i];
		}

		if (bufi >= KEYMAP_MACRO_LEN) {
			INIH_ERR("macro '%s' too long", macro);
			return false;
		}
	}

	if (mode == 1) {
		INIH_ERR("missing '>' while parsing macro: '%s'", macro);
		return false;
	}

	return true;
}

static bool
set_keymap(struct keymap *km, char *err, const char *c, const char *v, enum key_command kc)
{
	int tk, nk;
	uint8_t trigger_i = 0, consumed;
	char trigger_buf[KEYMAP_MACRO_LEN] = { 0 };

	tk = parse_cfg_keymap_key(c, &consumed);
	if (!consumed) {
		INIH_ERR("key is empty");
		return false;
	}
	c += consumed;

	while (1) {
		nk = parse_cfg_keymap_key(c, &consumed);
		if (!consumed) {
			break;
		}
		c += consumed;

		if (tk > ASCII_RANGE) {
			INIH_ERR("'%c' (%d) is outside of ascii range", tk, tk);
			return false;
		}

		trigger_buf[trigger_i++] = tk;
		if (trigger_i >= KEYMAP_MACRO_LEN - 1) {
			INIH_ERR("trigger too long");
			return false;
		}

		km = &km->map[tk];

		if (km->map == NULL) {
			keymap_init(km);
		}

		tk = nk;
	}

	trigger_buf[trigger_i++] = tk;
	km->map[tk].cmd = kc;

	if (kc == kc_macro) {
		if (!parse_macro(err, km->map[tk].strcmd, v)) {
			return false;
		}
	}

	strncpy(km->map[tk].trigger, trigger_buf, KEYMAP_MACRO_LEN);
	//km->map[tk].trigger_len = trigger_i;

	return true;
}

struct parse_keymap_ctx {
	struct keymap *km;
	struct ui_ctx *ui_ctx;
};

static bool
parse_keymap_handler(void *_ctx, char *err, const char *sec, const char *k, const char *v, uint32_t line)
{
	struct parse_keymap_ctx *ctx = _ctx;
	struct keymap *km = ctx->km;
	int32_t im, kc;

	if (sec == NULL) {
		INIH_ERR("mapping in without section not allowed. While parsing keymap at line %d", line);
		return false;
	}

	switch (ui_keymap_hook(ctx->ui_ctx, ctx->km, err, sec, k, v, line)) {
	case khr_matched:
		return true;
	case khr_unmatched:
		break;
	case khr_failed:
		return false;
	}

	if ((im = cfg_string_lookup(sec, &ltbl[table_im])) == -1) {
		LOG_I("skipping unmatched section '%s' line %d", sec, line);
		return true;
	}

	assert(k != NULL);
	assert(v != NULL);

	if ((kc = cfg_string_lookup(v, &key_command_ltbl)) == -1) {
		kc = kc_macro;
	}

	if (!(set_keymap(&km[im], err, k, v, kc))) {
		return false;
	}

	return true;
}

bool
parse_keymap(struct keymap *km, struct ui_ctx *ui_ctx)
{
	struct parse_keymap_ctx ctx = { .km = km, .ui_ctx = ui_ctx };

	return parse_cfg_file(KEYMAP_CFG, &ctx, parse_keymap_handler);
}
