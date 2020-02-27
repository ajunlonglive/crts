#include <string.h>

#include "client/display/window.h"
#include "client/display/world.h"
#include "client/graphics.h"
#include "client/hiface.h"
#include "shared/sim/action.h"
#include "shared/sim/chunk.h"
#include "shared/sim/ent.h"
#include "shared/sim/world.h"
#include "shared/util/log.h"

struct world_composite {
	struct rectangle ref;
	struct pixel **layers;
	struct pixel **composite;
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

struct pixel px_empty = { '_', color_blk };

#define LAYER_INDEX(x, y, z) (z * wcomp.ref.width * wcomp.ref.height) + (y * wcomp.ref.width) + x
#define CLAYER_INDEX(x, y) (y * wcomp.ref.width) + x
#define INVALID_POS(p, w) p.x < 0 || p.x >= w->ref.width || p.y < 0 || p.y >= w->ref.height

static void
write_chunk(struct world_composite *wc, const struct chunk *ck)
{
	struct graphics_info_t *tileg;
	struct point p, cp, rp = point_sub(&ck->pos, &wc->ref.pos);

	for (cp.x = 0; cp.x < CHUNK_SIZE; ++cp.x) {
		for (cp.y = 0; cp.y < CHUNK_SIZE; ++cp.y) {
			p = point_add(&cp, &rp);

			if (INVALID_POS(p, wc)) {
				continue;
			}

			tileg = &graphics.tiles[ck->tiles[cp.x][cp.y]];
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

static bool
write_ents(struct world_composite *wc, const struct world *w)
{
	bool written = false;
	struct graphics_info_t *entg;
	struct point p;
	size_t i;

	for (i = 0; i < w->ents.len; i++) {
		p = point_sub(&w->ents.e[i].pos, &wc->ref.pos);

		if (INVALID_POS(p, wc)) {
			continue;
		}

		entg = &graphics.ents[w->ents.e[i].type];
		wc->layers[LAYER_INDEX(p.x, p.y, entg->zi)] = &entg->pix;
		written = true;
	}

	return written;
};

static bool
update_composite(const struct win *win, const struct world_composite *wc)
{
	bool wrote = false;
	int z;
	size_t k;
	struct pixel *px;
	struct point cp;

	for (cp.x = 0; cp.x < wc->ref.width; ++cp.x) {
		for (cp.y = 0; cp.y < wc->ref.height; ++cp.y) {
			px = &px_empty;

			for (z = z_index_count - 1; z >= 0; --z) {
				k = LAYER_INDEX(cp.x, cp.y, z);

				if (wc->layers[k] != NULL) {
					px = wc->layers[k];
					break;
				}
			}

			k = CLAYER_INDEX(cp.x, cp.y);

			if (px != wc->composite[k]) {
				wrote = true;
				wc->composite[k] = px;
				win_write_px(win, &cp, px);
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

	wc->composite = realloc(wc->composite, wc->layer_size);
	memset(wc->composite, 0, wc->layer_size);
}

bool
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
		/*
		 * Not necessary? write chunks should fill every space
		 * memset(wcomp.layers, zi_0, wcomp.layer_size);
		 */

		commit |= write_chunks(&wcomp, hf->sim->w->chunks);
	}

	if (redraw || hf->sim->changed.ents) {
		memset(&wcomp.layers[zi_1 * wcomp.layer_len], 0, wcomp.layer_size * 2);

		commit |= write_ents(&wcomp, hf->sim->w);
	}

	if (commit) {
		return update_composite(win, &wcomp);
	}

	return commit;
}
