#include "sim/ent.h"
#include "sim/alignment.h"
#include "sim/world.h"
#include "window.h"
#include "painters.h"

void draw_infol(struct win *win, struct point *view, struct point *cursor)
{
	struct point p = { 0, 0 };

	win_printf(win, &p, "simlation running");
	p.y++;
	win_printf(win, &p, "view: (%d, %d) | cursor: (%d, %d)",
		   view->x, view->y,
		   cursor->x + view->x,
		   cursor->y + view->y);
}

void draw_infor(struct win *win, struct world *w)
{
	struct point p = { 0, 0 };

	win_printf(win, &p, "total entities: %d", w->ecnt);
}

void draw_world(struct win *win, struct world *w, struct point *view)
{
	size_t i;
	struct point np;
	enum color clr;

	for (i = 0; i < w->ecnt; i++) {
		np = w->ents[i].pos;

		np.x -= view->x;
		np.y -= view->y;

		clr = w->ents[i].alignment->max == 0 ? color_wte : color_grn;

		set_color(clr);
		win_write(win, &np, '@');
		unset_color(clr);
	}

	unset_color(color_grn);

};

void draw_selection(struct win *win, struct point *cursor)
{
	set_color(color_red);
	win_write(win, cursor, '$');
	unset_color(color_red);
}
