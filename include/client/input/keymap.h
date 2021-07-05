#ifndef CLIENT_INPUT_KEYMAP_H
#define CLIENT_INPUT_KEYMAP_H

#define ASCII_RANGE 128
#define KEYMAP_MACRO_LEN 32
#define KEYMAP_DESC_LEN 64

#include <stddef.h>

#include "shared/util/inih.h"

enum key_command {
	kc_none,
	kc_center_cursor,
	kc_macro,
	kc_invalid,
	kc_view_left,
	kc_view_down,
	kc_view_up,
	kc_view_right,
	kc_find,
	kc_set_input_mode,
	kc_quit,
	kc_cursor_left,
	kc_cursor_down,
	kc_cursor_up,
	kc_cursor_right,
	kc_set_action_type,
	kc_pause,

	/* debugging */
	kc_debug_pathfind_toggle,
	kc_debug_pathfind_place_point,

	key_command_count
};

enum input_mode {
	im_normal,
	im_cmd,
	im_none,
	input_mode_count = 2
};

extern const char *input_mode_names[input_mode_count];

#define KC_MACRO_MAX_NODES 32
#define EXPR_ARG_MAX 2

enum kc_macro_node_type {
	kcmnt_char,
	kcmnt_expr,
};

struct kc_node {
	union {
		char c;
		struct {
			enum key_command kc;
			int32_t argv[EXPR_ARG_MAX - 1];
			uint8_t argc;
		} expr;
	} val;
	uint8_t type;
};

struct kc_macro {
	struct kc_node node[KC_MACRO_MAX_NODES];
	uint8_t nodes;
};

struct keymap {
	char trigger[KEYMAP_MACRO_LEN];
	char desc[KEYMAP_DESC_LEN + 1];
	struct keymap *map;
	struct kc_macro cmd;
};


enum keymap_category {
	kmc_dont_use = 0,
	kmc_nav,
	kmc_sys,
	kmc_debug,
};

enum keymap_hook_result {
	khr_failed,
	khr_unmatched,
	khr_matched
};

enum cmd_string_lookup_table {
	cslt_commands,
	cslt_constants,
	cmd_string_lookup_table_count
};

extern const struct cfg_lookup_table
	cmd_string_lookup_tables[cmd_string_lookup_table_count];

void keymap_init(struct keymap *km);
#endif
