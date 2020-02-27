#define _XOPEN_SOURCE 500

#include <curses.h>
#include <locale.h>
#include <stdlib.h>
#include <string.h>

#include "client/display/container.h"
#include "client/display/window.h"
#include "client/draw.h"
#include "client/input/handler.h"
#include "client/net/receive.h"
#include "client/net/respond.h"
#include "client/request_missing_chunks.h"
#include "client/sim.h"
#include "client/world_update.h"
#include "shared/sim/world.h"
#include "shared/types/queue.h"
#include "shared/util/log.h"
#include "shared/util/time.h"

#define TICK NS_IN_S / 30

char default_addr[] = "127.0.0.1";

int
main(int argc, const char **argv)
{
	setlocale(LC_ALL, "");

	struct simulation sim = {
		.w = world_init(),
		.inbound = queue_init(),
		.outbound = queue_init(),
		.run = 1,
		.actions = {
			.e = NULL,
			.len = 0,
			.cap = 0
		}
	};
	struct server_cx scx;
	struct timespec tick_st;
	struct display_container dc;
	struct keymap *km;
	struct hiface *hif;
	int key;
	long slept_ns = 0;

	// connect to server
	server_cx_init(&scx, argc < 2 ? default_addr : argv[1]);
	scx.inbound  = sim.inbound;
	scx.outbound = sim.outbound;

	net_respond_init();
	net_receive_init();

	term_setup();
	dc_init(&dc);

	hif = hiface_init(&sim);
	km = &hif->km[hif->im];

	request_missing_chunks_init();

	L("beginning main loop");
	clock_gettime(CLOCK_REALTIME, &tick_st);

	while (hif->sim->run) {
		net_receive(&scx);

		memset(&sim.changed, 0, sizeof(sim.changed));
		world_update(&sim);

		draw(&dc, hif);

		if ((key = getch()) != ERR) {
			if ((km = handle_input(km, key, hif)) == NULL) {
				km = &hif->km[hif->im];
			}
		}

		request_missing_chunks(hif, &dc.root.world->rect);

		net_respond(&scx);

		slept_ns = sleep_remaining(&tick_st, TICK, slept_ns);
	}

	L("shutting down");
	term_teardown();

	return 0;
}
