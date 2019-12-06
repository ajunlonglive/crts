#include "sim/ent.h"
#include "sim/alignment.h"
#include "sim/world.h"
#include "window.h"
#include "painters.h"
#include "sim/chunk.h"
#include "constants/tile_chars.h"
#include "util/log.h"

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

const enum color tile_clr[] = {
	[tile_empty] = color_bg_cyn,
	[tile_full]  = color_bg_ylw,
	[tile_a]     = color_bg_blu,
	[tile_b]     = color_bg_mag,
	[tile_c]     = color_bg_wte
};

static void draw_chunk(struct win *win, struct point *view, struct chunk *ck)
{
	if (ck == NULL)
		return;

	struct point np = point_sub(&ck->pos, view);
	int onpy = np.y;
	int i, j;

	for (i = 0; i < CHUNK_SIZE; np.x++, i++)
		for ((np.y = onpy), (j = 0); j < CHUNK_SIZE; np.y++, j++) {
			if (ck->tiles[i][j] > 5) {
				win_write(win, &np, '!');
			} else {
				set_color(tile_clr[ck->tiles[i][j]]);
				win_write(win, &np, tile_chars[ck->tiles[i][j]]);
				unset_color(tile_clr[ck->tiles[i][j]]);
			}
		}
}

void draw_world(struct win *win, struct world *w, struct point *view)
{
	size_t i;
	struct point np = { view->x - (view->x % CHUNK_SIZE), 0 };
	int onpy = view->y - (view->y % CHUNK_SIZE);
	enum color clr;
	struct rectangle *r = &win->rect;

	for (; np.x < view->x + r->width; np.x += CHUNK_SIZE)
		for (np.y = onpy; np.y < view->y + r->height; np.y += CHUNK_SIZE)
			draw_chunk(win, view, hash_get(w->chunks, &np));


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
