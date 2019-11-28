#include "state.h"
#include "world.h"
#include "window.h"
#include "drawers.h"

struct state gs;

void draw_infol(struct win *win)
{
	struct point p = { 0, 0 };

	win_printf(win, &p, "simlation running");
	p.y++;
	win_printf(win, &p, "view: (%d, %d) | cursor: (%d, %d)",
		   gs.view.x, gs.view.y,
		   gs.cursor.x + gs.view.x,
		   gs.cursor.y + gs.view.y);
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
	enum color clr;

	for (i = 0; i < gs.w->ecnt; i++) {
		np = gs.w->ents[i].pos;

		np.x -= gs.view.x;
		np.y -= gs.view.y;

		clr = gs.w->ents[i].alignment->max == 0 ? color_wte : color_grn;

		set_color(clr);
		win_write(win, &np, '@');
		unset_color(clr);
	}

	unset_color(color_grn);

	set_color(color_red);
	if (gs.mode == view_mode_select)
		win_write(win, &gs.cursor, '$');
	unset_color(color_red);
};
