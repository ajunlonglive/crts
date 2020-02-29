#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "client/cfg/ini.h"
#include "client/cfg/keymap.h"
#include "client/input/keycodes.h"
#include "client/input/modes.h"
#include "shared/util/log.h"

static const struct {
	char *str;
	enum key_command kc;
} str_to_key_command[] = {
	{ "none",                 kc_none                 },
	{ "invalid",              kc_invalid              },
	{ "center",               kc_center               },
	{ "view_left",            kc_view_left            },
	{ "view_down",            kc_view_down            },
	{ "view_up",              kc_view_up              },
	{ "view_right",           kc_view_right           },
	{ "enter_selection_mode", kc_enter_selection_mode },
	{ "enter_normal_mode",    kc_enter_normal_mode    },
	{ "quit",                 kc_quit                 },
	{ "cursor_left",          kc_cursor_left          },
	{ "cursor_down",          kc_cursor_down          },
	{ "cursor_up",            kc_cursor_up            },
	{ "cursor_right",         kc_cursor_right         },
	{ "action_move",          kc_action_move          },
	{ "action_harvest",       kc_action_harvest       },
	{ "action_build",         kc_action_build         },
};

static enum key_command
s2kc(const char *str)
{
	int i;

	for (i = 0; i < key_command_count; i++) {
		if (strcmp(str_to_key_command[i].str, str) == 0) {
			return str_to_key_command[i].kc;
		}
	}

	return kc_invalid;
}

static const struct {
	char *str;
	enum input_mode im;
} str_to_input_mode[] = {
	{ "normal",  im_normal  },
	{ "select",  im_select  },
};

static enum input_mode
s2im(const char *str)
{
	int i;

	for (i = 0; i < input_mode_count; i++) {
		if (strcmp(str_to_input_mode[i].str, str) == 0) {
			return str_to_input_mode[i].im;
		}
	}

	return im_invalid;
}

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

static void
alloc_keymap(struct keymap *km)
{
	km->map = calloc(ASCII_RANGE, sizeof(struct keymap));
}

enum keymap_error {
	ke_ok,
	ke_invalid_char,
	ke_empty_key
};

static int
set_keymap(struct keymap *km, const char *c, enum key_command kc)
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
			alloc_keymap(km);
		}

		tk = nk;
	}

	km->map[tk].cmd = kc;

	return 0;
}

static int
parser_handler(void *vp, const char *sect, const char *k, const char *v, int line)
{
	struct keymap *km = vp;
	enum input_mode im;
	enum key_command kc;
	enum keymap_error ke;

	if (sect == NULL || (im = s2im(sect)) == im_invalid) {
		L("invalid input mode '%s' while parsing keymap at line %d", sect, line);
		return 0;
	} else if (v == NULL || (kc = s2kc(v)) == kc_invalid) {
		L("invalid key command '%s' while parsing keymap at line %d", v, line);
		return 0;
	} else if (k == NULL || (ke = set_keymap(&km[im], k, kc)) != ke_ok) {
		L("invalid keymap '%s' = '%s' while parsing keymap at line %d", k, v, line);
		return 0;
	}

	return 1;
}

static struct keymap *
ini_master_keymap(void)
{
	int i;
	struct keymap *km;

	km = calloc(input_mode_count, sizeof(struct keymap));

	for (i = 0; i < input_mode_count; i++) {
		alloc_keymap(&km[i]);
	}

	return km;
}

struct keymap *
parse_keymap(const char *filename)
{
	struct keymap *km = ini_master_keymap();

	if (access(filename, R_OK) != 0) {
		L("keymap '%s' not found", filename);
		km = NULL;
	} else {
		L("parsing keymap '%s'", filename);
	}

	if (ini_parse(filename, parser_handler, km) < 0) {
		L("error parsing keymap '%s'", filename);
		km = NULL;
	}

	return km;
}
