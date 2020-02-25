#include "client/display/painters.h"
#include "client/display/window.h"
#include "client/graphics.h"
#include "client/hiface.h"
#include "shared/sim/alignment.h"
#include "shared/sim/chunk.h"
#include "shared/sim/ent.h"
#include "shared/sim/world.h"
#include "shared/util/log.h"

void
draw_infol(struct win *win, struct hiface *hif)
{
	struct point p = { 0, 0 };

	win_printf(win, &p, "simlation running");
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

static void
draw_chunk(struct win *win, struct point *view, const struct chunk *ck)
{
	if (ck == NULL) {
		return;
	}

	struct point np = point_sub(&ck->pos, view);
	int onpy = np.y;
	int i, j;

	for (i = 0; i < CHUNK_SIZE; np.x++, i++) {
		for ((np.y = onpy), (j = 0); j < CHUNK_SIZE; np.y++, j++) {
			if (ck->tiles[i][j] >= 5 || ck->tiles[i][j] < 0) {
				win_write_g(win, &np, &graphics.cursor);
			} else {
				win_write_g(win, &np, &graphics.tiles[ck->tiles[i][j]]);
			}
		}
	}
}

void
draw_world(struct win *win, struct world *w, struct point *view)
{
	size_t i;
	struct point onp, np = onp = nearest_chunk(view);
	struct rectangle *r = &win->rect;
	const struct chunk *cnk;

	for (; np.x < view->x + r->width; np.x += CHUNK_SIZE) {
		for (np.y = onp.y; np.y < view->y + r->height; np.y += CHUNK_SIZE) {
			if ((cnk = hdarr_get(w->chunks->hd, &np)) != NULL) {
				draw_chunk(win, view, cnk);
			}
		}
	}

	for (i = 0; i < w->ents.len; i++) {
		np = w->ents.e[i].pos;

		np.x -= view->x;
		np.y -= view->y;

		win_write_g(win, &np, &graphics.ents[w->ents.e[i].type]);
		//clr = w->ents[i].alignment->max == 0 ? color_wte : color_grn;
	}

	unset_color(color_grn);

};

void
draw_selection(struct win *win, struct point *cursor)
{
	set_color(color_red);
	win_write(win, cursor, '$');
	unset_color(color_red);
}

void
draw_actions(struct win *win, struct action *a, size_t len, struct point *view)
{
	size_t i;
	struct point p;

	set_color(color_wte);

	for (i = 0; i < len; i++) {
		p = point_sub(&a[i].range.center, view);
		win_write(win, &p, '!');
	}

	unset_color(color_wte);
}
