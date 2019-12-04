#include <time.h>

#include "container.h"
#include "painters.h"
#include "window.h"
#include "../sim.h"
#include "display.h"
#include "math/geom.h"

#define FPS 30

static void fix_cursor(struct rectangle *r, struct point *vu, struct point *cursor)
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
	int key, run = 1;
	struct display_container dc;
	struct point cursor = { 0, 0 };
	struct point view = { 0, 0 };
	struct timespec tick = { 0, 1000000000 / FPS };

	term_setup();
	dc_init(&dc);

	while (run) {
		handle_input();
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

