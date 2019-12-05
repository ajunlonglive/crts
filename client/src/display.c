#define _POSIX_C_SOURCE 201900L

#include <time.h>
#include <curses.h>

#include "display/container.h"
#include "display/painters.h"
#include "display/window.h"
#include "display.h"
#include "input/handler.h"
#include "cfg/keymap.h"
#include "math/geom.h"
#include "util/log.h"

#define FPS 30

static void fix_cursor(const struct rectangle *r, struct point *vu, struct point *cursor)
{
	int diff;

	if (point_in_rect(cursor, r))
		return;

	if ((diff = 0 - cursor->y) < 0 || (diff = cursor->y - r->height) < 0) {
		vu->y -= diff;
		cursor->y -= 0;
	}
}

void display(struct simulation *sim)
{
	int key;
	struct display_container dc;
	struct point cursor = { 0, 0 };
	struct point view = { 0, 0 };
	struct timespec tick = { 0, 1000000000 / FPS };
	struct keymap *km, *rkm;

	term_setup();
	dc_init(&dc);

	rkm = km = parse_keymap("defcfg/keymap.ini");

	while (sim->run) {
		if ((key = getch()) != ERR)
			if ((km = handle_input(km, key, sim)) == NULL)
				km = rkm;

		fix_cursor(&dc.root.world->rect, &view, &cursor);

		win_erase();
		draw_infol(dc.root.info.l, &view, &cursor);
		draw_infor(dc.root.info.r, sim->w);
		draw_world(dc.root.world, sim->w, &view);
		//draw_selection();
		win_refresh();
		nanosleep(&tick, NULL);
	}

	term_teardown();
}
