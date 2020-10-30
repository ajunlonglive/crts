#include "posix.h"

#include <locale.h>
#include <string.h>

#include "client/cfg/keymap.h"
#include "client/input/handler.h"
#include "client/net.h"
#include "client/opts.h"
#include "client/request_missing_chunks.h"
#include "client/sim.h"
#include "client/ui/common.h"
#include "shared/serialize/to_disk.h"
#include "shared/sim/world.h"
#include "shared/util/assets.h"
#include "shared/util/log.h"
#include "shared/util/time.h"

#define TICK NS_IN_S / 30

int
main(int argc, char * const *argv)
{
	setlocale(LC_ALL, "");

	log_init();

	struct c_simulation sim = { .w = world_init(), .run = 1, };
	struct c_opts opts = { 0 };
	struct net_ctx *nx = NULL;
	struct timespec tick_st;
	struct keymap *km;
	struct hiface *hif;
	struct rectangle viewport;
	long slept_ns = 0;

	process_c_opts(argc, argv, &opts);
	sim.id = opts.id;

	if (opts.load_map
	    && load_world_from_path(opts.load_map, &sim.w->chunks)) {
		/* empty */
	} else if ((nx = net_init(&sim))) {
		set_server_address(opts.ip_addr);

		request_missing_chunks_init();
	} else {
		return 1;
	}

	struct ui_ctx ui_ctx;
	ui_init(&opts, &ui_ctx);

	hif = hiface_init(&sim);
	hif->nx = nx;
	hif->ui_ctx = &ui_ctx;
	km = &hif->km[hif->im];

	if (!parse_keymap(hif->km, &ui_ctx)) {
		return 1;
	}

	LOG_I("client initialized");
	clock_gettime(CLOCK_MONOTONIC, &tick_st);

	if (opts.cmds) {
		/* HACK: this is because the first time render is called, the
		 * opengl ui overwrites the camera position making goto
		 * commands ineffective */
		ui_render(&ui_ctx, hif);
		run_cmd_string(hif, opts.cmds);
	}

	while (hif->sim->run) {
		memset(&sim.changed, 0, sizeof(sim.changed));
		hif->next_act_changed = false;
		hif->input_changed = false;

		if (nx) {
			check_add_server_cx(nx);
			request_missing_chunks(hif, &viewport, nx);
			send_msgs(nx);
			recv_msgs(nx);
		}

		ui_handle_input(&ui_ctx, &km, hif);

		ui_render(&ui_ctx, hif);

		viewport = ui_viewport(&ui_ctx);

		slept_ns = sleep_remaining(&tick_st, TICK, slept_ns);
	}

	L("shutting down");
	ui_deinit(&ui_ctx);

	return 0;
}
