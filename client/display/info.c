#include <curses.h>

#include "client/display/info.h"
#include "client/display/window.h"
#include "client/hiface.h"
#include "shared/sim/world.h"

void
draw_infol(struct win *win, struct hiface *hif)
{
	struct point p = { 0, 0 };

	win_printf(win, &p, "redraws: (+%6d) | ping: (+%6uf)", hif->redrew_world,
		hif->server_timeout);
	clrtoeol();
	p.y++;
	win_printf(win, &p, "view: (%d, %d) | cursor: (%d, %d)",
		hif->view.x, hif->view.y,
		hif->cursor.x + hif->view.x,
		hif->cursor.y + hif->view.y);
	clrtoeol();
	p.y++;
	win_printf(win, &p, "cmd: %s%s", hif->num.buf, hif->cmd.buf);
	clrtoeol();
}

void
draw_infor(struct win *win, struct world *w)
{
	struct point p = { 0, 0 };

	win_printf(win, &p, "ents : % ld, chunks:% ld ", hdarr_len(w->ents), hdarr_len(w->chunks->hd));
	clrtoeol();
}
