#define _XOPEN_SOURCE 500

#include <curses.h>
#include <locale.h>
#include <stdlib.h>
#include <string.h>

#include "client/cfg/cfg.h"
#include "client/display/container.h"
#include "client/display/window.h"
#include "client/draw.h"
#include "client/input/handler.h"
#include "client/net.h"
#include "client/opts.h"
#include "client/request_missing_chunks.h"
#include "client/sim.h"
#include "client/world_update.h"
#include "shared/messaging/client_message.h"
#include "shared/net/net_ctx.h"
#include "shared/sim/world.h"
#include "shared/util/log.h"
#include "shared/util/time.h"

#define TICK NS_IN_S / 30

int
main(int argc, char * const *argv)
{
	setlocale(LC_ALL, "");

	struct simulation sim = { .w = world_init(), .run = 1, };
	struct opts opts = { 0 };
	struct net_ctx *nx;
	struct timespec tick_st;
	struct display_container dc;
	struct keymap *km;
	struct hiface *hif;
	int key;
	long slept_ns = 0;

	process_opts(argc, argv, &opts);

	nx = net_init(opts.ip_addr);
	net_set_outbound_id(opts.id);

	term_setup();

	hif = hiface_init(&sim);
	hif->nx = nx;
	km = &hif->km[hif->im];

	if (!parse_all_cfg(&opts, &graphics, hif->km)) {
		return 1;
	}

	init_tile_curs();
	dc_init(&dc);

	request_missing_chunks_init();

	L("beginning main loop");
	clock_gettime(CLOCK_REALTIME, &tick_st);

	while (hif->sim->run) {
		net_receive(nx);

		memset(&sim.changed, 0, sizeof(sim.changed));
		world_update(&sim, nx);

		draw(&dc, hif);

		hif->next_act_changed = false;
		if ((key = getch()) != ERR) {
			if ((km = handle_input(km, key, hif)) == NULL) {
				km = &hif->km[hif->im];
			}
		}

		request_missing_chunks(hif, &dc.root.world->rect, nx);

		send_msg(nx, client_message_poke, NULL, msgf_forget);
		net_respond(nx);

		slept_ns = sleep_remaining(&tick_st, TICK, slept_ns);
	}

	L("shutting down");
	term_teardown();

	return 0;
}
