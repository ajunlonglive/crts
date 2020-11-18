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

	struct world w = { 0 };
	world_init(&w);
	struct c_simulation sim = { .w = &w, .run = 1, };
	struct c_opts opts = { 0 };
	struct net_ctx nx = { 0 };
	struct timespec tick_st;
	struct keymap *km;
	struct hiface hf = { 0 };
	struct rectangle viewport;
	long slept_ns = 0;
	bool online = false;

	process_c_opts(argc, argv, &opts);
	sim.id = opts.id;

	if (opts.load_map
	    && load_world_from_path(opts.load_map, &sim.w->chunks)) {
		/* empty */
	} else {
		online = true;
		net_init(&sim, &nx);
		set_server_address(opts.ip_addr);

		request_missing_chunks_init();
	}

	struct ui_ctx ui_ctx;
	ui_init(&opts, &ui_ctx);

	hiface_init(&hf, &sim);
	hf.nx = &nx;
	hf.ui_ctx = &ui_ctx;
	km = &hf.km[hf.im];

	if (!parse_keymap(hf.km, &ui_ctx)) {
		return 1;
	}

	LOG_I("client initialized");
	clock_gettime(CLOCK_MONOTONIC, &tick_st);

	if (opts.cmds) {
		/* HACK: this is because the first time render is called, the
		 * opengl ui overwrites the camera position making goto
		 * commands ineffective */
		ui_render(&ui_ctx, &hf);
		run_cmd_string(&hf, opts.cmds);
	}

	while (hf.sim->run) {
		memset(&sim.changed, 0, sizeof(sim.changed));
		hf.next_act_changed = false;
		hf.input_changed = false;

		if (online) {
			check_add_server_cx(&nx);
			request_missing_chunks(&hf, &viewport, &nx);
			send_msgs(&nx);
			recv_msgs(&nx);
		}

		ui_handle_input(&ui_ctx, &km, &hf);

		ui_render(&ui_ctx, &hf);

		viewport = ui_viewport(&ui_ctx);

		slept_ns = sleep_remaining(&tick_st, TICK, slept_ns);
	}

	L("shutting down");
	ui_deinit(&ui_ctx);

	return 0;
}
