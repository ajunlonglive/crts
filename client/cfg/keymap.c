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

#define MAX_KEY_NAME_LEN 32
static struct cfg_lookup_table special_keys = {
	"up",        skc_up,
	"down",      skc_down,
	"left",      skc_left,
	"right",     skc_right,
	"enter",     '\n',
	"tab",       '\t',
	"space",     ' ',
	"home",      skc_home,
	"end",       skc_end,
	"page_up",   skc_pgup,
	"page_down", skc_pgdn,
};

static uint8_t
parse_cfg_keymap_key(const char *str, uint8_t *consumed, char *err)
{
	uint32_t i;
	int16_t val;

	switch (str[0]) {
	case '\\':
		if (str[1]) {
			*consumed = 2;
			return str[1];
		} else {
			INIH_ERR("incomplete '\\' escape");
			*consumed = 0;
			return 0;
		}
	case '<':
		for (i = 0; str[i]; ++i) {
			if (str[i] == '>') {
				break;
			}
		}

		if (!str[i]) {
			INIH_ERR("missing >");
			*consumed = 0;
			return 0;
		}

		if (i >= MAX_KEY_NAME_LEN) {
			INIH_ERR("key name too long");
			*consumed = 0;
			return 0;
		}

		char buf[MAX_KEY_NAME_LEN + 1] = { 0 };
		memcpy(buf, &str[1], i - 1);

		if ((val = cfg_string_lookup(buf, &special_keys)) == -1) {
			INIH_ERR("invalid key name: '%s'", buf);
			*consumed = 0;
			return 0;
		}

		*consumed = i + 2;
		return val;
	case 0:
		INIH_ERR("null character");
		*consumed = 0;
		return 0;
	default:
		*consumed = 1;
		return str[0];
	}
}

#define EXPR_MAX 32

enum parse_macro_mode {
	pmm_default,
	pmm_expr,
};

#define EXPR_BEG '{'
#define EXPR_END '}'
#define EXPR_ARG_SEP ':'

static bool
parse_macro(char *err, struct kc_macro *kcm, const char *macro)
{
	enum parse_macro_mode mode = pmm_default;
	uint32_t i;
	uint8_t j;
	int32_t val;
	char *endptr;

	struct {
		struct {
			char chars[EXPR_MAX + 1];
			uint32_t i;
		} buf[EXPR_ARG_MAX];

		uint8_t argc;
	} expr = { 0 };
	uint8_t consumed, c;

	for (i = 0; macro[i]; ++i) {
		assert(macro[i]);
		switch (mode) {
		case pmm_expr:
			switch (macro[i]) {
			case ' ': case '\t':
				break;
			case EXPR_END:
				assert(expr.buf[expr.argc].i < EXPR_MAX);

				expr.buf[expr.argc].chars[expr.buf[expr.argc].i] = 0;
				++expr.argc;

				const struct cfg_lookup_table *tbl = &cmd_string_lookup_tables[cslt_commands];

				for (j = 0; j < expr.argc; ++j) {
					if ((val = cfg_string_lookup(expr.buf[j].chars, tbl)) != -1) {
						/* nothing */
					}  else if ((val = strtol(expr.buf[j].chars, &endptr, 10)) && !*endptr) {
						/* nothing */
					} else {
						INIH_ERR("invalid expression literal: '%s' while parsing macro: '%s'",
							expr.buf[j].chars, macro);
						return false;
					}

					if (j == 0) {
						kcm->node[kcm->nodes].val.expr.kc = val;

						tbl = &cmd_string_lookup_tables[cslt_constants];
					} else {
						kcm->node[kcm->nodes].val.expr.argv[j - 1] = val;
					}
				}

				kcm->node[kcm->nodes].type = kcmnt_expr;
				kcm->node[kcm->nodes].val.expr.argc = expr.argc - 1;
				++kcm->nodes;

				if (kcm->nodes >= KC_MACRO_MAX_NODES) {
					INIH_ERR("macro '%s' too long", macro);
					return false;
				}

				mode = pmm_default;
				break;
			case EXPR_ARG_SEP:
				expr.buf[expr.argc].chars[expr.buf[expr.argc].i] = 0;
				++expr.argc;

				if (expr.argc > EXPR_ARG_MAX) {
					INIH_ERR("too many expression args");
					return false;
				}

				break;
			default:
				expr.buf[expr.argc].chars[expr.buf[expr.argc].i] = macro[i];
				++expr.buf[expr.argc].i;


				if (expr.buf[expr.argc].i >= EXPR_MAX) {
					INIH_ERR("const '%s' too long while parsing macro: '%s'",
						expr.buf[expr.argc].chars, macro);
					return false;
				}
			}
			break;
		case pmm_default:
			switch (macro[i]) {
			case '\\':
				goto parse_char;
				break;
			case EXPR_BEG:
				memset(&expr, 0, sizeof(expr));
				mode = pmm_expr;
				break;
			default:
parse_char:
				c = parse_cfg_keymap_key(&macro[i], &consumed, err);
				if (!consumed) {
					return false;
				}

				kcm->node[kcm->nodes].type = kcmnt_char;
				kcm->node[kcm->nodes].val.c = c;
				++kcm->nodes;

				if (consumed > 1) {
					i += consumed - 1;
				}

				if (kcm->nodes >= KC_MACRO_MAX_NODES) {
					INIH_ERR("macro '%s' too long", macro);
					return false;
				}
			}
		}
	}

	if (mode != pmm_default) {
		INIH_ERR("unconsumed tokens while parsing macro: '%s'", macro);
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

	tk = parse_cfg_keymap_key(c, &consumed, err);
	if (!consumed) {
		return false;
	}
	c += consumed;

	while (1) {
		nk = parse_cfg_keymap_key(c, &consumed, err);
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

	if (kc == kc_macro) {
		if (!parse_macro(err, &km->map[tk].cmd, v)) {
			return false;
		}
	} else {
		km->map[tk].cmd.node[0].type = kcmnt_expr;
		km->map[tk].cmd.node[0].val.expr.kc = kc;
		km->map[tk].cmd.nodes = 1;
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

	if (sec != NULL) {
		switch (ui_keymap_hook(ctx->ui_ctx, ctx->km, err, sec, k, v, line)) {
		case khr_matched:
			return true;
		case khr_unmatched:
			LOG_W("unknown keymap section: '%s'", sec);
			return true;
		case khr_failed:
			return false;
		}
	}

	im = im_normal;
	/* if ((im = cfg_string_lookup(sec, &ltbl[table_im])) == -1) { */
	/* 	LOG_I("skipping unmatched section '%s' line %d", sec, line); */
	/* 	return true; */
	/* } */

	assert(k != NULL);
	assert(v != NULL);

	if ((kc = cfg_string_lookup(v, &cmd_string_lookup_tables[cslt_commands])) == -1) {
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
