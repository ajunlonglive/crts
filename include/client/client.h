#ifndef CLIENT_HIFACE_H
#define CLIENT_HIFACE_H

#include <stdint.h>

#include "client/input/cmdline.h"
#include "client/input/keymap.h"
#include "client/opts.h"
#include "shared/msgr/msgr.h"
#include "shared/sound/sound.h"
#include "shared/util/timer.h"

#ifndef NDEBUG
#include "shared/pathfind/api.h"
#endif

#define INPUT_BUF_LEN 32

enum client_state_flags {
	csf_view_initialized = 1 << 1,
	csf_paused           = 1 << 2,
};

struct client_buf {
	char buf[INPUT_BUF_LEN];
	size_t len;
};

typedef void (*tick_func)(struct client *cli);

struct client {
	struct hash requested_chunks;

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

	/* view position and cursor */
	struct point cursor, view;
	struct rectangle viewport;

	/* changed switches */
	struct { bool chunks, ents, input; } changed;

	/* actions */
	enum action action;

	/* big pointers */
	struct world *world;
	struct msgr *msgr;
	struct ui_ctx *ui_ctx;
	struct sound_ctx sound_ctx;

	/* debugging */
#ifndef NDEBUG
	struct {
		bool on;
		uint32_t path;
		struct point goal;
		struct darr path_points;
	} debug_path;
#endif

	struct {
		struct timer_avg client_tick;
		float server_fps;
	} prof;

	/* TODO: remove? */
	uint32_t redrew_world;
	bool sound_triggered;
};

bool init_client(struct client *cli, struct client_opts *opts);
void deinit_client(struct client *cli);
void client_tick(struct client *cli);
#endif
