#include "posix.h"

#include <curses.h>

#include "client/hiface.h"
#include "client/ui/ncurses/info.h"
#include "client/ui/ncurses/window.h"
#include "shared/constants/globals.h"
#include "shared/sim/world.h"
#include "shared/util/log.h"

static void
draw_cli(struct win *win, struct hiface *hf)
{
	struct point p = { 0, 0 };

	p.x = win_printf(win, &p, ":%s", hf->cmdline.cur.buf);
	win_clrtoeol(win, &p);
	p.x = 1 + hf->cmdline.cur.cursor;

	struct pixel curs = graphics.cursor[ct_default].pix;
	curs.c = ' ';

	if (hf->cmdline.cur.cursor < hf->cmdline.cur.len) {
		curs.c = hf->cmdline.cur.buf[hf->cmdline.cur.cursor];
	}

	win_write_px(win, &p, &curs);
	win_clr_attr();

	p.x = 0;

	int32_t i;
	for (i = 0; i < win->rect.height; ++i) {
		if (*hf->cmdline.history.in[i]) {
			p.x = 0;
			++p.y;
			p.x = win_printf(win, &p, ":%s", hf->cmdline.history.in[i]);
			win_clrtoeol(win, &p);

			if (*hf->cmdline.history.out[i]) {
				p.x = 0;
				++p.y;
				p.x = win_printf(win, &p, "%s", hf->cmdline.history.out[i]);
				win_clrtoeol(win, &p);
			}
		} else {
			++p.y;
			win_clrtoeol(win, &p);
		}
	}
}

void
draw_infol(struct win *win, struct hiface *hif)
{
	if (hif->im == im_cmd) {
		draw_cli(win, hif);
		return;
	}

	struct point p = { 0, 0 };
	const char *act_tgt_nme;

	p.x += win_printf(win, &p, "redraws: (+%6d) | cmd: %5.5s%5.5s",
		hif->redrew_world, hif->num.buf, hif->cmd.buf);
	win_clrtoeol(win, &p);

	p.x = 0;
	p.y++;
	p.x = win_printf(win, &p,
		"view: (%4d, %4d) | cursor: (%4d, %4d) | rect: %dx%d",
		hif->view.x, hif->view.y, hif->cursor.x + hif->view.x,
		hif->cursor.y + hif->view.y,
		hif->next_act.range.width,
		hif->next_act.range.height
		);
	win_clrtoeol(win, &p);

	switch (hif->next_act.type) {
	case at_build:
		act_tgt_nme = gcfg.tiles[hif->next_act.tgt].name;
		break;
	default:
		act_tgt_nme = "";
		break;
	}
	p.x = 0;
	p.y++;
	p.x = win_printf(win, &p, "act: %s%c %s, %x",
		gcfg.actions[hif->next_act.type].name,
		*act_tgt_nme ? ',' : ' ', act_tgt_nme,
		hif->next_act.flags);
	win_clrtoeol(win, &p);

	/*
	   p.x = 0;
	   p.y++;
	   p.x = win_printf(win, &p, "motiv: %3d, ents : % 5ld, chunks:% 5ld ",
	        hif->sim->assigned_motivator,
	        hdarr_len(hif->sim->w->ents), hdarr_len(hif->sim->w->chunks->hd));
	   win_clrtoeol(win, &p);
	 */
}

struct ent_count_ctx {
	uint32_t *ec;
	uint32_t *tc;
	const struct point *p;
};

static enum iteration_result
ent_counter(void *_ctx, void *_e)
{
	struct ent_count_ctx *ctx = _ctx;
	struct ent *e = _e;

	++ctx->tc[e->type];

	if (points_equal(&e->pos, ctx->p)) {
		++(ctx->ec[e->type]);
	}

	return ir_cont;
}

static void
print_ent_counts(struct win *win, struct hiface *hif, struct point *vp, struct point *cp)
{
	uint32_t ent_counts[ent_type_count] = { 0 };
	uint32_t tent_counts[ent_type_count] = { 0 };
	struct ent_count_ctx ctx = { ent_counts, tent_counts, cp };

	hdarr_for_each(&hif->sim->w->ents, &ctx, ent_counter);

	vp->x = win_printf(win, vp, "w: %d / %d | @: %d / %d",
		ent_counts[et_resource_wood], tent_counts[et_resource_wood],
		ent_counts[et_worker], tent_counts[et_worker]);
	win_clrtoeol(win, vp);
}

void
draw_infor(struct win *win, struct hiface *hif)
{
	static struct point op = { 1, 1 };
	struct point vp = { 0, 0 };
	struct point cp = point_add(&hif->cursor, &hif->view);
	struct chunk *ck;

	if (hif->sim->changed.ents) {
		print_ent_counts(win, hif, &vp, &cp);
	}

	vp.x = 0;
	++vp.y;

	if (!points_equal(&op, &cp)) {
		op = nearest_chunk(&cp);

		if ((ck = hdarr_get(&hif->sim->w->chunks.hd, &op)) != NULL) {
			op = point_sub(&cp, &op);

			vp.x = win_printf(win, &vp, "tile: %-20.20s",
				gcfg.tiles[ck->tiles[op.x][op.y]].name);
			win_clrtoeol(win, &vp);
		}

		op = cp;
	}
}
