#include "posix.h"

#include <assert.h>
#include <string.h>

#include "client/hiface.h"
#include "client/ui/ncurses/graphics.h"
#include "client/ui/ncurses/window.h"
#include "client/ui/ncurses/world.h"
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

struct pixel px_empty = { 0, 0, '_' };

#define LAYER_INDEX(x, y, z) ((z) * wcomp.ref.width * wcomp.ref.height) + \
	((y) * wcomp.ref.width) + (x)

static bool
pos_is_invalid(struct world_composite *wc, struct point *p)
{
	return p->x < 0 || p->x >= wc->ref.width || p->y < 0 || p->y >= wc->ref.height;
}

static void
check_write_graphic(struct world_composite *wc, struct point *p, struct graphics_info_t *g)
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

			t = ck == NULL ? tile_deep_water : ck->tiles[cp.x][cp.y];
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
			write_chunk(wc, hdarr_get(cnks->hd, &sp), &sp);
		}
	}

	return true;
}

struct write_ent_ctx {
	struct world_composite *wc;
	const struct c_simulation *sim;
};

static enum iteration_result
write_ent(void *_ctx, void *_e)
{
	struct write_ent_ctx *ctx = _ctx;
	struct ent *e = _e;
	struct graphics_info_t *entg;
	struct point p;
	uint32_t ent_type;

	if (e->type == et_none) {
		return ir_cont;
	}

	p = point_sub(&e->pos, &ctx->wc->ref.pos);

	if (pos_is_invalid(ctx->wc, &p)) {
		return ir_cont;
	}

	if ((ent_type = e->type) == et_worker) {
		if (e->alignment == ctx->sim->assigned_motivator) {
			ent_type = et_elf_friend;
		} else {
			ent_type = et_elf_foe;
		}
	}

	entg = &graphics.entities[ent_type];
	ctx->wc->layers[LAYER_INDEX(p.x, p.y, entg->zi)] = &entg->pix;

	return ir_cont;
}

static bool
write_ents(struct world_composite *wc, const struct c_simulation *sim)
{
	struct write_ent_ctx ctx = { wc, sim };

	hdarr_for_each(sim->w->ents, &ctx, write_ent);

	return true;
}

static void
write_crosshair(struct world_composite *wc, const struct rectangle *r,
	const struct point *p, enum cursor_type t)
{
	struct point np = *p;

	check_write_graphic(wc, &np, &graphics.cursor[t]);

	np.x += r->width - 1;
	check_write_graphic(wc, &np, &graphics.cursor[t]);

	np.x -= r->width - 1;
	np.y += r->height - 1;
	check_write_graphic(wc, &np, &graphics.cursor[t]);

	np.x += r->width - 1;
	check_write_graphic(wc, &np, &graphics.cursor[t]);
}

static void
write_blueprint(struct world_composite *wc, struct chunks *cnks,
	const struct point *view, const struct point *p,
	const struct rectangle *r)
{
	struct point cp, vp, rp, q;
	struct chunk *ck;
	enum tile ct;

	for (q.x = r->pos.x; q.x < r->pos.x + r->width; ++q.x) {
		for (q.y = r->pos.y; q.y < r->pos.y + r->height; ++q.y) {
			vp = point_add(p, &q);
			rp = point_add(view, &vp);
			cp = nearest_chunk(&rp);

			if ((ck = hdarr_get(cnks->hd, &cp))) {
				cp = point_sub(&rp, &ck->pos);
				ct = ck->tiles[cp.x][cp.y];
			} else {
				ct = 0;
			}

			check_write_graphic(wc, &vp,
				gcfg.tiles[ct].foundation
				? &graphics.cursor[ct_blueprint_valid]
				: &graphics.cursor[ct_blueprint_invalid]);
		}
	}
}

static void
write_harvest_tgt(struct world_composite *wc, struct chunks *cnks,
	const struct point *curs, const struct point *view,
	const struct rectangle *r)
{
	struct point p, cp, q;
	struct chunk *ck;


	for (q.x = 0; q.x < r->width; ++q.x) {
		for (q.y = 0; q.y < r->height; ++q.y) {
			p = point_add(curs, view);
			p = point_add(&q, &p);

			cp = nearest_chunk(&p);

			if (!(ck = hdarr_get(cnks->hd, &cp))) {
				continue;
			}

			cp = point_sub(&p, &ck->pos);

			if (gcfg.tiles[ck->tiles[cp.x][cp.y]].hardness) {
				cp = point_sub(&p, view);

				check_write_graphic(wc, &cp, &graphics.cursor[ct_default]);
			}
		}
	}
}

static void
write_action(struct world_composite *wc, const struct hiface *hf,
	const struct action *act, bool new)
{
	struct point c, q;
	enum cursor_type crosshair;

	if (new) {
		c = hf->cursor;
		crosshair = ct_crosshair;
	} else {
		c = point_sub(&act->range.pos, &hf->view);
		crosshair = ct_crosshair_dim;
	}

	switch (act->type) {
	case at_harvest:
		write_crosshair(wc, &act->range, &c, crosshair);

		write_harvest_tgt(wc, hf->sim->w->chunks, &c, &hf->view, &act->range);
		break;
	case at_build:
		write_blueprint(wc, hf->sim->w->chunks, &hf->view, &c, &act->range);

		write_crosshair(wc, &act->range, &c, crosshair);
		break;
	case at_fight:
		write_crosshair(wc, &act->range, &c, crosshair);

		check_write_graphic(wc, &c, &graphics.cursor[ct_default]);
		break;
	case at_carry:
		q = point_sub(&act->source.pos, &hf->view);
		write_crosshair(wc, &act->source, &q, ct_crosshair_dim);

		write_crosshair(wc, &act->range, &c, crosshair);

		check_write_graphic(wc, &c, &graphics.ent_curs[act->tgt]);
		break;
	default:
		check_write_graphic(wc, &c, &graphics.cursor[ct_default]);
		break;
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
	if (!(redraw || hf->sim->changed.actions || oim != hf->im ||
	      ((hf->im == im_select || hf->im == im_resize)
	       && (!points_equal(&oc, &c) || hf->next_act_changed)))) {
		goto skip_write_selection;
	}

	wrote = true;
	memset(&wc->layers[zi_3 * wcomp.layer_len], 0, wcomp.layer_size);

	if (hf->im != im_select && hf->im != im_resize) {
		goto skip_write_selection;
	}

	size_t i;
	for (i = 0; i < ACTION_HISTORY_SIZE; ++i) {
		if (hf->sim->action_history[i].type) {
			write_action(wc, hf, &hf->sim->action_history[i], false);
		}
	}

	/* Write the current selection on on top of everything */
	write_action(wc, hf, &hf->next_act, true);

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

		commit |= write_ents(&wcomp, hf->sim);
	}

	commit |= write_selection(&wcomp, hf, redraw);

	if (commit) {
		return update_composite(win, &wcomp);
	}

	return commit;
}
