#include <assert.h>
#include <string.h>

#include "client/display/window.h"
#include "client/display/world.h"
#include "client/graphics.h"
#include "client/hiface.h"
#include "shared/constants/globals.h"
#include "shared/sim/action.h"
#include "shared/sim/chunk.h"
#include "shared/sim/ent.h"
#include "shared/sim/world.h"
#include "shared/util/log.h"

struct world_composite {
	struct rectangle ref;
	struct pixel **layers;
	struct pixel *composite;
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

struct pixel px_empty = { '_', 0, 0 };

#define LAYER_INDEX(x, y, z) ((z) * wcomp.ref.width * wcomp.ref.height) + ((y) * wcomp.ref.width) + (x)
#define CLAYER_INDEX(x, y) ((y) * wcomp.ref.width) + (x)
#define INVALID_POS(p, w) p.x < 0 || p.x >= w->ref.width || p.y < 0 || p.y >= w->ref.height
#define WRITE_GRAPHIC(wc, x, y, gr) wc->layers[LAYER_INDEX(x, y, gr.zi)] = &gr.pix
#define PXEQUAL(p1, p2) (p1.c == p2.c && p1.attr == p2.attr && p1.clr == p2.clr)

static void
write_chunk(struct world_composite *wc, const struct chunk *ck)
{
	struct graphics_info_t *tileg;
	struct point p, cp, rp = point_sub(&ck->pos, &wc->ref.pos);
	enum tile t;

	for (cp.x = 0; cp.x < CHUNK_SIZE; ++cp.x) {
		for (cp.y = 0; cp.y < CHUNK_SIZE; ++cp.y) {
			p = point_add(&cp, &rp);

			if (INVALID_POS(p, wc)) {
				continue;
			}

			t = ck->tiles[cp.x][cp.y];
			assert(t < tile_count);

			tileg = &graphics.tiles[t];
			wc->layers[LAYER_INDEX(p.x, p.y, tileg->zi)] = &tileg->pix;
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

	const struct chunk *ck;

	for (; sp.x < endx; sp.x += CHUNK_SIZE) {
		for (sp.y = spy; sp.y < endy; sp.y += CHUNK_SIZE) {
			if ((ck = hdarr_get(cnks->hd, &sp)) != NULL) {
				write_chunk(wc, ck);
			}
		}
	}

	return true;
}

static enum iteration_result
write_ent(void *_wc, void *_e)
{
	struct world_composite *wc = _wc;
	struct ent *e = _e;
	struct graphics_info_t *entg;
	struct point p;

	if (e->type == et_none) {
		return ir_cont;
	}

	p = point_sub(&e->pos, &wc->ref.pos);

	if (INVALID_POS(p, wc)) {
		return ir_cont;
	}

	entg = &graphics.ents[e->type];
	wc->layers[LAYER_INDEX(p.x, p.y, entg->zi)] = &entg->pix;

	return ir_cont;
}

static bool
write_ents(struct world_composite *wc, const struct world *w)
{
	hdarr_for_each(w->ents, wc, write_ent);

	return true;
};

static void
write_crosshair(struct world_composite *wc, const struct circle *c, const struct point *p)
{
	WRITE_GRAPHIC(wc, p->x + c->r, p->y, graphics.arrow.right);
	WRITE_GRAPHIC(wc, p->x - c->r, p->y, graphics.arrow.left);
	WRITE_GRAPHIC(wc, p->x, p->y + c->r, graphics.arrow.down);
	WRITE_GRAPHIC(wc, p->x, p->y - c->r, graphics.arrow.up);
}

static void
write_blueprint(struct world_composite *wc, enum building blpt, const struct point *p)
{
	size_t i;
	const struct blueprint *bp = &gcfg.blueprints[blpt];
	struct point rp;

	for (i = 0; i < bp->len; ++i) {
		rp = point_add(p, &bp->blocks[i].p);
		WRITE_GRAPHIC(wc, rp.x, rp.y, graphics.blueprint);
	}
}

static bool
write_selection(struct world_composite *wc, const struct hiface *hf, bool redraw)
{
	static struct point oc = { 0, 0 };
	struct point c = hf->cursor;
	static enum input_mode oim = im_normal;
	bool wrote = false;

	/* lots of conditions
	 * - we have a forced redraw
	 * - the old input method was select but it changed
	 * - the input method is select and
	 *   - the cursor got moved or the action's params were changed
	 */
	if (!(redraw || oim != hf->im ||
	      (hf->im == im_select &&
	       (!points_equal(&oc, &hf->cursor) || hf->next_act_changed)))) {
		goto skip_write_selection;
	}

	memset(&wc->layers[zi_3 * wcomp.layer_len], 0, wcomp.layer_size);

	if (hf->im != im_select) {
		wrote = true;
		goto skip_write_selection;
	}

	switch (hf->next_act.type) {
	case at_harvest:
		write_crosshair(wc, &hf->next_act.range, &hf->cursor);

		WRITE_GRAPHIC(wc, c.x, c.y, graphics.harvest[hf->next_act.tgt]);
		break;
	case at_build:
		write_blueprint(wc, hf->next_act.tgt, &c);
		break;
	default:
		WRITE_GRAPHIC(wc, c.x, c.y, graphics.cursor);
		break;
	}

skip_write_selection:
	oim = hf->im;
	oc = c;
	return wrote;
}


static uint32_t
update_composite(const struct win *win, const struct world_composite *wc)
{
	uint32_t wrote = 0;
	int z;
	size_t k;
	struct pixel cpx;
	struct point cp;

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

			k = CLAYER_INDEX(cp.x, cp.y);

			if (!PXEQUAL(cpx, wc->composite[k])) {
				wrote++;
				wc->composite[k] = cpx;
				win_write_px(win, &cp, &cpx);
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

	wc->layers = realloc(wc->layers, wc->total_size);
	memset(wc->layers, 0, wc->total_size);

	wc->composite = realloc(wc->composite, sizeof(struct pixel) * wc->layer_len);
	memset(wc->composite, 0, sizeof(struct pixel) * wc->layer_len);
}

uint32_t
draw_world(const struct win *win, const struct hiface *hf)
{

	bool commit = false, redraw = false;

	if (wcomp.ref.width != win->rect.width ||
	    wcomp.ref.height != win->rect.height) {
		resize_layers(&wcomp, &win->rect);
		redraw = true;
	}

	if (!points_equal(&wcomp.ref.pos, &hf->view)) {
		wcomp.ref.pos = hf->view;
		redraw = true;
	}

	if (redraw || hf->sim->changed.chunks) {
		commit |= write_chunks(&wcomp, hf->sim->w->chunks);
	}

	if (redraw || hf->sim->changed.ents) {
		memset(&wcomp.layers[zi_1 * wcomp.layer_len], 0, wcomp.layer_size * 2);

		commit |= write_ents(&wcomp, hf->sim->w);
	}

	commit |= write_selection(&wcomp, hf, redraw);

	if (commit) {
		return update_composite(win, &wcomp);
	}

	return commit;
}
