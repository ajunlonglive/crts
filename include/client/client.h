#ifndef CLIENT_HIFACE_H
#define CLIENT_HIFACE_H

#include <stdint.h>

#include "client/input/cmdline.h"
#include "client/input/keymap.h"
#include "client/opts.h"
#include "shared/msgr/msgr.h"

#ifndef NDEBUG
#include "shared/pathfind/api.h"
#endif

#define INPUT_BUF_LEN 32
#define ACTION_HISTORY_SIZE 256

enum client_state_flags {
	csf_display_help = 1 << 0,
	csf_view_initialized = 1 << 1,
};

struct client_buf {
	char buf[INPUT_BUF_LEN];
	size_t len;
};

typedef void (*tick_func)(struct client *cli);

struct client {
	/* misc */
	uint16_t id;
	tick_func tick;

	/* state flags */
	uint8_t state;
	bool run;

	/* input related buffers */
	struct client_buf num;
	struct { bool override; long val; } num_override;
	struct client_buf cmd;
	struct cmdline cmdline;

	/* keymaps */
	enum input_mode im;
	struct keymap keymaps[input_mode_count];
	struct keymap *ckm;

	/* keymap descriptions */
	bool keymap_describe;
	char description[KEYMAP_DESC_LEN];
	size_t desc_len;

	/* view position and cursor */
	struct point cursor, view;
	struct rectangle viewport;

	/* resizing */
	struct {
		struct point cntr, tmpcurs, oldcurs;
		bool b;
	} resize;

	/* changed switches */
	struct { bool chunks, ents, actions, input, next_act; } changed;

	/* actions */
	struct action next_act;
	uint8_t action_seq;
	struct action action_history[ACTION_HISTORY_SIZE];
	uint8_t action_history_order[ACTION_HISTORY_SIZE];
	size_t action_history_len;
	enum cursor_action curs_act;

	/* big pointers */
	struct world *world;
	struct msgr *msgr;
	struct ui_ctx *ui_ctx;

	/* debugging */
#ifndef NDEBUG
	struct {
		bool on;
		uint32_t path;
		struct point goal;
		struct darr path_points;
	} debug_path;
#endif

	/* TODO: remove? */
	uint32_t redrew_world;
};

bool init_client(struct client *cli, struct client_opts *opts);
void deinit_client(struct client *cli);
void client_tick(struct client *cli);
#endif
