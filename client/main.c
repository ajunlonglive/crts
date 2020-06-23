#include "posix.h"

#include <locale.h>
#include <string.h>

#include "client/cfg/common.h"
#include "client/cfg/keymap.h"
#include "client/input/handler.h"
#include "client/net.h"
#include "client/opts.h"
#include "client/request_missing_chunks.h"
#include "client/sim.h"
#include "client/ui/common.h"
#include "client/world_update.h"
#include "shared/messaging/client_message.h"
#include "shared/net/net_ctx.h"
#include "shared/sim/world.h"
#include "shared/util/assets.h"
#include "shared/util/log.h"
#include "shared/util/time.h"

#define TICK NS_IN_S / 30

int
main(int argc, char * const *argv)
{
	setlocale(LC_ALL, "");

	struct c_simulation sim = { .w = world_init(), .run = 1, };
	struct c_opts opts = { 0 };
	struct net_ctx *nx;
	struct timespec tick_st;
	struct keymap *km;
	struct hiface *hif;
	struct rectangle viewport;
	long slept_ns = 0;

	process_c_opts(argc, argv, &opts);
	asset_path_init(opts.asset_path);

	struct ui_ctx *ui_ctx;
	ui_ctx = ui_init(&opts);

	nx = net_init(opts.ip_addr);
	net_set_outbound_id(opts.id);

	hif = hiface_init(&sim);
	hif->nx = nx;
	km = &hif->km[hif->im];

	if (!parse_keymap(km)) {
		return 1;
	}

	request_missing_chunks_init();

	LOG_I("client initialized");
	clock_gettime(CLOCK_MONOTONIC, &tick_st);

	while (hif->sim->run) {
		check_add_server_cx(nx);
		net_receive(nx);

		memset(&sim.changed, 0, sizeof(sim.changed));
		hif->next_act_changed = false;
		hif->input_changed = false;

		world_update(&sim, nx);

		ui_handle_input(ui_ctx, &km, hif);

		ui_render(ui_ctx, hif);

		viewport = ui_viewport(ui_ctx);
		request_missing_chunks(hif, &viewport, nx);

		send_msg(nx, client_message_poke, NULL, msgf_forget);
		net_respond(nx);

		slept_ns = sleep_remaining(&tick_st, TICK, slept_ns);
	}

	L("shutting down");
	ui_deinit(ui_ctx);

	return 0;
}
