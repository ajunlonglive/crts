#ifndef CLIENT_HIFACE_H
#define CLIENT_HIFACE_H

#include <stdint.h>

#include "client/cmdline.h"
#include "client/opts.h"
#include "client/ui/common.h"
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

enum input_mode {
	im_normal,
	im_cmd,
	input_mode_count,
};

typedef void (*tick_func)(struct client *cli);

struct client {
	struct hash requested_chunks;
	struct hash ents;

	/* misc */
	uint16_t id;
	tick_func tick;

	/* state flags */
	uint8_t state;
	bool run;

	/* input related */
	enum input_mode im;
	struct cmdline cmdline;

	/* view position and cursor */
	struct point cursor;
	struct pointf cursorf;
	float cursor_z;
	struct {
		struct pointf center;
		float angle, h, w;
		struct rect rect;
	} ref;

	/* changed switches */
	struct { bool chunks, ents, input; } changed;

	/* actions */
	bool do_action, do_action_once;
	enum action action;
	uint16_t action_arg;

	/* big pointers */
	struct world *world;
	struct msgr *msgr;
	struct ui_ctx *ui_ctx;
	struct client_opts *opts;

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
bool reset_client(struct client *cli, struct client_opts *opts);
void deinit_client(struct client *cli);
void client_tick(struct client *cli);
#endif
