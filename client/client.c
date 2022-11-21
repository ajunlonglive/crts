#include "posix.h"

#include <math.h>
#include <string.h>

#include "client/client.h"
#include "client/handle_msg.h"
#include "client/input_cfg.h"
#include "client/input_handler.h"
#include "client/opts.h"
#include "client/ui/common.h"
#include "shared/constants/numbers.h"
#include "shared/math/rand.h"
#include "shared/util/log.h"
#include "tracy.h"

#define REQUEST_COOLDOWN 3

bool
reset_client(struct client *cli, struct client_opts *opts)
{
	world_reset(cli->world);
	hash_clear(&cli->requested_chunks);
	cli->run = true;
	cli->im = im_normal;
	cli->state = 0;

	cli->changed.chunks = true;
	cli->changed.input = true;
	cli->changed.ents = true;

	cli->ref.h = 64;
	cli->ref.w = 64;
	make_rect(&(struct pointf){ 256, 256 }, 64, 64, &cli->ref.rect);

	ui_reset(cli);
	return true;
}

bool
init_client(struct client *cli, struct client_opts *opts)
{
	cli->opts = opts;

	if (!opts->id) {
		opts->id = rand_uniform(0xffff - MIN_USER_ID) + MIN_USER_ID;
	} else if (opts->id <= MIN_USER_ID) {
		LOG_W(log_misc, "invalid client id: %u, correcting to %u", opts->id, opts->id + MIN_USER_ID);
		opts->id += MIN_USER_ID;
	}

	L(log_misc, "client id: %u", opts->id);

	if (opts->ui == ui_default) {
#ifdef OPENGL_UI
		L(log_misc, "using default ui: gl");
		opts->ui = ui_gl;
#else
#ifdef NCURSES_UI
		L(log_misc, "using default ui: term");
		opts->ui = ui_term;
#else
		L(log_misc, "using default ui: null");
		opts->ui = ui_null;
#endif
#endif
	}

	if (!opts->sound.disable) {
		if (!sound_init(opts->sound.device)) {
			LOG_W(log_misc, "failed to initialize sound");
		}

		sound_trigger_3d((vec3){ 200.0, 50.0, 250.0 }, audio_asset_theme_1, audio_flag_loop);
		sound_trigger_3d((vec3){ 500.0, 50.0, 300.0 }, audio_asset_theme_2, audio_flag_loop);
		sound_trigger_3d((vec3){ 350.0, 50.0, 600.0 }, audio_asset_theme_3, audio_flag_loop);

		sound_set_val(sound_volume_master, opts->sound.master);
		sound_set_val(sound_volume_music, opts->sound.music);
		sound_set_val(sound_volume_sfx, opts->sound.sfx);
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

	input_init();

	if (!input_cfg_parse(opts->keymap)) {
		return false;
	}

	cli->im = im_normal;

#ifndef NDEBUG
	darr_init(&cli->debug_path.path_points, sizeof(struct point));
#endif

	hash_init(&cli->requested_chunks, 2048, sizeof(struct point));
	hash_init(&cli->ents, 2048, sizeof(struct point3d));

	if (opts->cmds) {
		run_cmd_string(cli, opts->cmds);
	}

	cli->state |= csf_view_initialized;
	/* cli->cursor = (struct point) { 300, 300 }; */
	/* make_rect(&(struct pointf){ 256, 256 }, 64, 64, &cli->ref); */
	cli->cursorf = (struct pointf) { 256, 256 };

	cli->ref.h = 256;
	cli->ref.w = 256;
	cli->ref.angle = D2R(90.0f);
	cli->ref.center = cli->cursorf;
	make_rotated_rect(&cli->ref.center, cli->ref.h, cli->ref.w, cli->ref.angle, &cli->ref.rect);

	L(log_misc, "client initialized");
	return true;
}

void
deinit_client(struct client *cli)
{
	ui_deinit(cli->ui_ctx);
}

static void
request_missing_chunks(struct client *cli)
{
	struct rect containing_rect;
	containing_axis_aligned_rect(&cli->ref.rect, &containing_rect);
	struct point onp, np = onp = nearest_chunk(&(struct point) {
		containing_rect.p[0].x, containing_rect.p[0].y
	});

	for (; np.x < containing_rect.p[0].x + containing_rect.w; np.x += CHUNK_SIZE) {
		for (np.y = onp.y; np.y < containing_rect.p[0].y + containing_rect.h; np.y += CHUNK_SIZE) {
			if (hdarr_get(&cli->world->chunks.hd, &np) ||
			    np.x < 0 || np.y < 0
			    || hash_get(&cli->requested_chunks, &np)) {
				continue;
			}

			struct msg_req msg = {
				.mt = rmt_chunk,
				.dat = { .chunk = np }
			};

			msgr_queue(cli->msgr, mt_req, &msg, 0, priority_normal);

			hash_set(&cli->requested_chunks, &np, 0);
		}
	}
}

void
client_tick(struct client *cli)
{
	TracyCZoneAutoS;

	request_missing_chunks(cli);

	struct point cursor = cli->cursor;
	if (cursor.x > 0 && cursor.y > 0) {
		msgr_queue(cli->msgr, mt_cursor, &(struct msg_cursor){
			.cursor = cursor,
			.cursor_z = cli->cursor_z,
			.action = cli->do_action ? cli->action : act_neutral,
			.action_arg = cli->action_arg,
			.once = cli->do_action_once,
		}, 0, priority_dont_resend);
	}
	cli->do_action = false;
	cli->do_action_once = false;

	msgr_send(cli->msgr);
	msgr_recv(cli->msgr);

	ui_handle_input(cli);
	ui_render(cli);
	constrain_cursor(cli);

	sound_update(*ui_cam_pos(cli), cli->ref.angle);
	cli->sound_triggered = false;

	memset(&cli->changed, 0, sizeof(cli->changed));

	TracyCZoneAutoE;
}
