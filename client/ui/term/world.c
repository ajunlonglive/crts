#include "posix.h"

#include <assert.h>
#include <string.h>

#include "client/client.h"
#include "client/ui/term/graphics.h"
#include "client/ui/term/world.h"
#include "shared/constants/globals.h"
#include "shared/sim/action.h"
#include "shared/sim/chunk.h"
#include "shared/sim/ent.h"
#include "shared/sim/world.h"
#include "shared/ui/term/window.h"
#include "shared/util/log.h"
#include "shared/util/mem.h"

struct world_composite {
	struct rectangle ref;
	struct term_pixel **layers;
	struct term_pixel *composite;
	size_t layer_len;
	size_t layer_size;
	size_t total_len;
	size_t total_size;
};

struct world_composite wcomp = {
	.ref = {
		.pos = { 0, 0 },
		.width = 0,
		.height = 0
	},
	.layers = NULL,
	.layer_len = 0,
	.total_len = 0,
	.total_size = 0
};

struct term_pixel px_empty = { 0, 0, '_' };

#define LAYER_INDEX(x, y, z) ((z) * wcomp.ref.width * wcomp.ref.height) + \
	((y) * wcomp.ref.width) + (x)

static bool
pos_is_invalid(struct world_composite *wc, const struct point *p)
{
	return p->x < 0 || p->x >= wc->ref.width || p->y < 0 || p->y >= wc->ref.height;
}

static void
check_write_graphic(struct world_composite *wc, const struct point *p, struct graphics_info_t *g)
{
	if (!pos_is_invalid(wc, p)) {
		wc->layers[LAYER_INDEX(p->x, p->y, g->zi)] = &g->pix;
	}
}

static void
write_chunk(struct world_composite *wc, const struct chunk *ck, struct point *kp)
{
	struct point p, cp, rp = point_sub(kp, &wc->ref.pos);
	enum tile t;

	for (cp.x = 0; cp.x < CHUNK_SIZE; ++cp.x) {
		for (cp.y = 0; cp.y < CHUNK_SIZE; ++cp.y) {
			p = point_add(&cp, &rp);

			t = ck == NULL ? tile_sea : ck->tiles[cp.x][cp.y];
			assert(t < tile_count);

			check_write_graphic(wc, &p, &graphics.tiles[t]);
		}
	}
}

static bool
write_chunks(struct world_composite *wc, const struct chunks *cnks)
{
	struct point sp = nearest_chunk(&wc->ref.pos);
	int spy = sp.y,
	    endx = wc->ref.pos.x + wc->ref.width,
	    endy = wc->ref.pos.y + wc->ref.height;

	for (; sp.x < endx; sp.x += CHUNK_SIZE) {
		for (sp.y = spy; sp.y < endy; sp.y += CHUNK_SIZE) {
			write_chunk(wc, hdarr_get(&cnks->hd, &sp), &sp);
		}
	}

	return true;
}

static bool
write_ents(struct world_composite *wc, const struct client *cli)
{
	struct graphics_info_t *entg;
	struct point p;
	uint32_t i;

	struct ent *e;

	for (i = 0; i < hdarr_len(&cli->world->ents); ++i) {
		e = hdarr_get_by_i(&cli->world->ents, i);

		p = point_sub(&e->pos, &wc->ref.pos);

		if (pos_is_invalid(wc, &p)) {
			continue;
		}

		entg = &graphics.entities[e->type];
		wc->layers[LAYER_INDEX(p.x, p.y, entg->zi)] = &entg->pix;
	}

	return true;
}

static bool
write_selection(struct world_composite *wc, const struct client *cli, bool redraw)
{
	static struct point oc = { 0, 0 };
	struct point c = cli->cursor;
	static enum input_mode oim = im_normal;
	bool wrote = false;

	/* lots of conditions
	 * - we have a forced redraw
	 * - the old input method was select but it changed
	 * - the input method is select and
	 *   - the cursor got moved
	 */
	if (!(redraw || oim != cli->im ||
	      ((!points_equal(&oc, &c))))) {
		goto skip_write_selection;
	}

	wrote = true;
	memset(&wc->layers[zi_3 * wcomp.layer_len], 0, wcomp.layer_size);

	/* Write the current selection on on top of everything */
	check_write_graphic(wc, &cli->cursor, &graphics.cursor[ct_default]);

skip_write_selection:
	oim = cli->im;
	oc = c;
	return wrote;
}

static uint32_t
update_composite(uint32_t win, const struct world_composite *wc)
{
	uint32_t wrote = 0;
	int z;
	size_t k;
	struct term_pixel cpx;
	struct point cp;
	int32_t bg;

	for (cp.x = 0; cp.x < wc->ref.width; ++cp.x) {
		for (cp.y = 0; cp.y < wc->ref.height; ++cp.y) {
			cpx = px_empty;

			for (z = z_index_count - 1; z >= 0; --z) {
				k = LAYER_INDEX(cp.x, cp.y, z);

				if (wc->layers[k] != NULL) {
					if (cpx.c == CHAR_TRANS) {
						cpx.c = wc->layers[k]->c;
					} else {
						cpx = *wc->layers[k];
					}

					if (cpx.c != CHAR_TRANS) {
						break;
					}
				}
			}

			if (cpx.c == CHAR_TRANS && z > 0) {
				cpx.c = wc->layers[LAYER_INDEX(cp.x, cp.y, (z - 1))]->c;
			}

			if (cpx.bg == TRANS_COLOR) {
				for (; z >= 0; --z) {
					k = LAYER_INDEX(cp.x, cp.y, z);
					if (wc->layers[k]
					    && (bg = wc->layers[k]->bg) != TRANS_COLOR) {
						cpx.clr = get_bg_pair(cpx.fg, bg);
						break;
					}
				}
			}

			k = cp.y * wc->ref.width + cp.x;

			if (!(cpx.c == wc->composite[k].c
			      && cpx.attr == wc->composite[k].attr
			      && cpx.clr == wc->composite[k].clr)) {
				wrote++;
				wc->composite[k] = cpx;
				term_write_px(win, &cp, &cpx);
			}
		}
	}

	return wrote;
}

static void
resize_layers(struct world_composite *wc, const struct rectangle *newrect)
{
	wc->ref.width  = newrect->width;
	wc->ref.height = newrect->height;

	wc->layer_len  = wc->ref.width * wc->ref.height;
	wc->layer_size = sizeof(struct pixel *) * wc->layer_len;
	wc->total_len  = wc->layer_len * z_index_count;
	wc->total_size = wc->layer_size * z_index_count;

	wc->layers = z_realloc(wc->layers, wc->total_size);
	memset(wc->layers, 0, wc->total_size);

	wc->composite = z_realloc(wc->composite, sizeof(struct term_pixel) * wc->layer_len);
	memset(wc->composite, 0, sizeof(struct term_pixel) * wc->layer_len);
}

uint32_t
draw_world(uint32_t win, const struct client *cli)
{

	bool commit = false, redraw = false;

	const struct rectangle *win_rect = term_win_rect(win);

	if (wcomp.ref.width != win_rect->width ||
	    wcomp.ref.height != win_rect->height) {
		resize_layers(&wcomp, win_rect);
		redraw = true;
	}

	if (!points_equal(&wcomp.ref.pos, &cli->view)) {
		wcomp.ref.pos = cli->view;
		redraw = true;
	}

	if (redraw || cli->changed.chunks) {
		commit |= write_chunks(&wcomp, &cli->world->chunks);
	}

	if (redraw || cli->changed.ents) {
		memset(&wcomp.layers[zi_1 * wcomp.layer_len], 0, wcomp.layer_size * 2);

		commit |= write_ents(&wcomp, cli);
	}

	commit |= write_selection(&wcomp, cli, redraw);

	if (commit) {
		return update_composite(win, &wcomp);
	}

	return commit;
}
