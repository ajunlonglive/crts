#include "posix.h"

#include <string.h>

#include "client/cfg/keymap.h"
#include "client/client.h"
#include "client/handle_msg.h"
#include "client/input/helpers.h"
#include "client/opts.h"
#include "client/request_missing_chunks.h"
#include "client/ui/common.h"
#include "shared/constants/numbers.h"
#include "shared/math/rand.h"
#include "shared/util/log.h"
#include "tracy.h"

bool
init_client(struct client *cli, struct client_opts *opts)
{

	if (!opts->id) {
		opts->id = rand_uniform(0xffff - MIN_USER_ID) + MIN_USER_ID;
	} else if (opts->id <= MIN_USER_ID) {
		LOG_W("invalid client id: %u, correcting to %u", opts->id, opts->id + MIN_USER_ID);
		opts->id += MIN_USER_ID;
	}

	L("client id: %u", opts->id);

	if (opts->ui == ui_default) {
#ifdef OPENGL_UI
		L("using default ui: opengl");
		opts->ui = ui_opengl;
#else
#ifdef NCURSES_UI
		L("using default ui: ncurses");
		opts->ui = ui_ncurses;
#else
		L("using default ui: null");
		opts->ui = ui_null;
#endif
#endif
	}

	if (!opts->mute) {
		if (!sound_init(&cli->sound_ctx)) {
			LOG_W("failed to initialize sound");
		}
	}

	cli->id = opts->id;
	cli->run = true;

	static struct msgr msgr = { 0 };
	msgr_init(&msgr, cli, client_handle_msg, cli->id);
	cli->msgr = &msgr;

	static struct world world = { 0 };
	world_init(&world);
	cli->world = &world;

	static struct ui_ctx ui_ctx = { 0 };
	ui_init(opts, &ui_ctx);
	cli->ui_ctx = &ui_ctx;

	size_t i;
	for (i = 0; i < input_mode_count; ++i) {
		keymap_init(&cli->keymaps[i]);
	}

	if (!parse_keymap(cli->keymaps, &ui_ctx)) {
		return false;
	}

	client_reset_input(cli);


	cli->im = im_normal;
	cli->ckm = &cli->keymaps[cli->im];

#ifndef NDEBUG
	darr_init(&cli->debug_path.path_points, sizeof(struct point));
#endif

	/* load_world_from_path(opts->load_map, &cli->world->chunks); */

	request_missing_chunks_init();

	if (opts->cmds) {
		run_cmd_string(cli, opts->cmds);
	}

	L("client initialized");
	return true;
}

void
deinit_client(struct client *cli)
{
	ui_deinit(cli->ui_ctx);
}

void
client_tick(struct client *cli)
{
	TracyCZoneAutoS;

	request_missing_chunks(cli);

	struct point cursor = point_add(&cli->view, &cli->cursor);
	msgr_queue(cli->msgr, mt_cursor, &(struct msg_cursor){
		.cursor = cursor,
		.action = cli->action
	}, 0, priority_dont_resend);

	msgr_send(cli->msgr);
	msgr_recv(cli->msgr);

	ui_handle_input(cli);
	ui_render(cli);

	sound_update(&cli->sound_ctx, *ui_cam_pos(cli));
	cli->sound_triggered = false;

	memset(&cli->changed, 0, sizeof(cli->changed));

	TracyCZoneAutoE;
}
