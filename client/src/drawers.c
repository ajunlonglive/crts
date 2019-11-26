#include "state.h"
#include "world.h"
#include "window.h"
#include "drawers.h"

struct state gs;

void draw_infol(struct win *win)
{
	struct point p = { 0, 0 };

	win_printf(win, &p, "simlation running");
}

void draw_infor(struct win *win)
{
	struct point p = { 0, 0 };

	win_printf(win, &p, "total entities: %d", gs.w->ecnt);
}

void draw_world(struct win *win)
{
	size_t i;
	struct point np;

	for (i = 0; i < gs.w->ecnt; i++) {
		np = gs.w->ents[i].pos;

		np.x += gs.view.x;
		np.y += gs.view.y;

		win_write(win, &np, '@');
	}

	if (gs.mode == view_mode_select)
		win_write(win, &gs.cursor, '$');
};
