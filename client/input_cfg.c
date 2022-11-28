#include "posix.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "client/input_handler.h"
#include "client/input_cfg.h"
#include "shared/sim/action.h"
#include "shared/sim/chunk.h"
#include "shared/util/inih.h"
#include "shared/util/log.h"

const char *input_mode_names[input_mode_count] = {
	[im_normal] = "normal",
	[im_cmd]    = "command",
};

static struct cfg_lookup_table special_keys = {
	"up",        skc_up,
	"down",      skc_down,
	"left",      skc_left,
	"right",     skc_right,
	"enter",     '\n',
	"tab",       '\t',
	"space",     ' ',
	"esc",       '\033',
	"home",      skc_home,
	"end",       skc_end,
	"page_up",   skc_pgup,
	"page_down", skc_pgdn,
	"mb1",       skc_mb1,
	"mb2",       skc_mb2,
};

struct parse_keymap_ctx {
	struct keymaps *kms;
	struct ui_ctx *ui_ctx;
};

#define BUF_SIZE 256

static bool
startswith(const char *pre, const char *str, const char **ret)
{
	uint32_t n = strlen(pre);

	if (strncmp(pre, str, n) == 0) {
		*ret = &str[n];
		return true;
	}

	return false;
}

static bool
parse_keymap_handler(void *_ctx, char *err, const char *sec, const char *k, const char *v, uint32_t line)
{
	const char *key, *val, *whitespace = " \t";
	int32_t keycode;
	struct keymap_layer *l;
	struct keymap *m = NULL;
	char k_buf[BUF_SIZE] = { 0 };
	char v_buf[BUF_SIZE] = { 0 };

	assert(k != NULL);
	assert(v != NULL);

	if (strlen(k) >= BUF_SIZE) {
		INIH_ERR("key to long '%s'", k);
		return false;
	}
	strcpy(k_buf, k);

	if (strlen(v) >= BUF_SIZE) {
		INIH_ERR("val to long '%s'", v);
		return false;
	}
	strcpy(v_buf, v);

	km_set_layer(0);

	key = strtok(k_buf, whitespace);
	assert(key);

	while (key) {
		uint8_t mod = 0, action = key_action_oneshot;
		const char *suff;

		if (startswith("+", key, &suff)) {
			action = key_action_press;
			key = suff;
		} else if (startswith("-", key, &suff)) {
			action = key_action_release;
			key = suff;
		} else if (startswith("*", key, &suff)) {
			action = key_action_held;
			key = suff;
		}

		while (true) {
			if (startswith("ctrl+", key, &suff)) {
				mod |= mod_ctrl;
				key = suff;
			} else if (startswith("shift+", key, &suff)) {
				mod |= mod_shift;
				key = suff;
			} else {
				break;
			}
		}

		if ((keycode = cfg_string_lookup(key, &special_keys)) == -1) {
			if (strlen(key) == 1) {
				keycode = key[0];
			} else {
				/* unknown key */
				INIH_ERR("unknown key '%s'", key);
				return false;
			}
		}

		l = km_current_layer();
		if (!(m = km_find_or_create_map(l, keycode, mod, action))) {
			INIH_ERR("too many keymaps");
			return false;
		}

		if (m->func) {
			INIH_ERR("conflicting keymap: %s = %s", k, v);
			return false;
		} else if (m->next_layer) {
			km_set_layer(m->next_layer);
		} else {
			km_add_layer(&m->next_layer);
		}

		key = strtok(NULL, whitespace);
	}

	val = strtok(v_buf, whitespace);
	assert(val);

	if (!input_command_name_lookup(val, &m->func)) {
		L(log_cli, "ignoring unknown keymap command '%s'", val);
		return true;
	}

	if ((val = strtok(NULL, whitespace))) {
		int64_t arg;
		if (!input_constant_lookup(val, &m->arg)) {
			char *endptr;
			arg = strtol(val, &endptr, 10);

			if (*endptr) {
				INIH_ERR("invalid command argument '%s'", val);
				return false;
			} else if (arg < 0 || arg > UINT32_MAX) {
				INIH_ERR("command argument %ld out of range", arg);
				return false;
			}

			m->arg = arg;
		}
	}

	if ((val = strtok(NULL, whitespace))) {
		INIH_ERR("invalid extraneous argument '%s'", val);
		return false;
	}

	return true;
}

bool
input_cfg_parse(const char *keymap_path)
{
	if (!keymap_path) {
		keymap_path = "keymap.ini";
	}

	if (!parse_cfg_file(keymap_path, NULL, parse_keymap_handler)) {
		LOG_W(log_misc, "failed to parse input config");
		return false;
	}

	km_set_layer(0); // since it might not be reset after parsing
	return true;
}
