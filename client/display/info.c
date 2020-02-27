#include "client/display/info.h"
#include "client/display/window.h"
#include "client/hiface.h"
#include "shared/sim/world.h"

void
draw_infol(struct win *win, struct hiface *hif)
{
	struct point p = { 0, 0 };

	win_printf(win, &p, "simlation running (%c)", hif->redrew_world ? '!' : '_');
	p.y++;
	win_printf(win, &p, "view: (%d, %d) | cursor: (%d, %d)",
		hif->view.x, hif->view.y,
		hif->cursor.x + hif->view.x,
		hif->cursor.y + hif->view.y);
	p.y++;
	win_printf(win, &p, "cmd: %s%s", hif->num.buf, hif->cmd.buf);
}

void
draw_infor(struct win *win, struct world *w)
{
	struct point p = { 0, 0 };

	win_printf(win, &p, "total entities: %d", w->ents.len);
}
