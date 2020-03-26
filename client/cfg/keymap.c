#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "client/cfg/cfg.h"
#include "client/cfg/keymap.h"
#include "client/input/keymap.h"
#include "shared/util/log.h"

enum keymap_error {
	ke_ok,
	ke_invalid_char,
	ke_empty_key
};

enum tables {
	table_keycmd,
	table_im,
};

static struct lookup_table ltbl[] = {
	[table_keycmd] = {
		"none", kc_none,
		"invalid", kc_invalid,
		"center", kc_center,
		"center_cursor", kc_center_cursor,
		"view_left", kc_view_left,
		"view_down", kc_view_down,
		"view_up", kc_view_up,
		"view_right", kc_view_right,
		"enter_selection_mode", kc_enter_selection_mode,
		"enter_normal_mode", kc_enter_normal_mode,
		"quit", kc_quit,
		"cursor_left", kc_cursor_left,
		"cursor_down", kc_cursor_down,
		"cursor_up", kc_cursor_up,
		"cursor_right", kc_cursor_right,
		"set_action_type", kc_set_action_type,
		"set_action_target", kc_set_action_target,
		"set_action_radius", kc_set_action_radius,
		"set_action_source", kc_set_action_source,
		"swap_cursor_with_source", kc_swap_cursor_with_source,
		"action_radius_expand", kc_action_radius_expand,
		"exec_action", kc_exec_action,
		"", kc_macro,
	},
	[table_im] = {
		"normal", im_normal,
		"select", im_select,
	}
};

static int
next_key(const char **str)
{
	int k;

	switch (k = *(++*str)) {
	case '\\':
		switch (k = *(++*str)) {
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
		default:
			return k;
		case '\0':
			return -1;
		}
	case '\0':
		return -1;
	default:
		return k;
	}
}

static int
set_keymap(struct keymap *km, const char *c, const char *v, enum key_command kc)
{
	int tk, nk;
	const char **cp = &c;

	(*cp)--;

	if ((tk = next_key(cp)) == -1) {
		return ke_empty_key;
	}

	while ((nk = next_key(cp)) != -1) {
		if (tk > ASCII_RANGE) {
			return ke_invalid_char;
		}

		km = &km->map[tk];

		if (km->map == NULL) {
			keymap_init(km);
		}

		tk = nk;
	}

	km->map[tk].cmd = kc;

	if (kc == kc_macro) {
		assert(strlen(v) < KEYMAP_MACRO_LEN);
		strncpy(km->map[tk].strcmd, v, KEYMAP_MACRO_LEN - 1);
	}

	return 0;
}

int
parse_keymap_handler(void *vp, const char *sec, const char *k, const char *v, int line)
{
	struct keymap *km = vp;
	enum keymap_error ke;
	int32_t im, kc;

	if (sec == NULL || (im = cfg_string_lookup(sec, &ltbl[table_im])) == -1) {
		L("invalid input mode '%s' while parsing keymap at line %d", sec, line);
		return 0;
	}

	assert(k != NULL);
	assert(v != NULL);

	if ((kc = cfg_string_lookup(v, &ltbl[table_keycmd])) == -1) {
		kc = kc_macro;
		L("binding %s to macro: %s", k, v);
	}

	if ((ke = set_keymap(&km[im], k, v, kc)) != ke_ok) {
		L("invalid keymap '%s' = '%s' while parsing keymap at line %d",
			k, v, line);
		return 0;
	}

	return 1;
}
