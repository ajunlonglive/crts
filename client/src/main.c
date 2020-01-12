#define _DEFAULT_SOURCE

#include <stdlib.h>
#include <locale.h>
#include <curses.h>

#include "draw.h"
#include "request_missing_chunks.h"
#include "display/window.h"
#include "display/container.h"
#include "input/handler.h"
#include "util/log.h"
#include "util/time.h"
#include "net/receive.h"
#include "net/respond.h"
#include "sim.h"
#include "sim/world.h"
#include "types/queue.h"
#include "world_update.h"

#define TICK NS_IN_S / 30

char default_addr[] = "127.0.0.1";

int main(int argc, const char **argv)
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

		world_update(&sim);

		draw(&dc, hif);

		if ((key = getch()) != ERR)
			if ((km = handle_input(km, key, hif)) == NULL)
				km = &hif->km[hif->im];

		request_missing_chunks(hif, &dc.root.world->rect);

		net_respond(&scx);

		slept_ns = sleep_remaining(&tick_st, TICK, slept_ns);
	}

	L("shutting down");
	term_teardown();

	return 0;
}
