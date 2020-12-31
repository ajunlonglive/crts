#include "posix.h"

#include <curses.h>

#include "client/client.h"
#include "client/ui/ncurses/info.h"
#include "client/ui/ncurses/window.h"
#include "shared/constants/globals.h"
#include "shared/sim/world.h"
#include "shared/util/log.h"

static void
draw_cli(struct win *win, struct client *cli)
{
	struct point p = { 0, 0 };

	p.x = win_printf(win, &p, ":%s", cli->cmdline.cur.buf);
	win_clrtoeol(win, &p);
	p.x = 1 + cli->cmdline.cur.cursor;

	struct pixel curs = graphics.cursor[ct_default].pix;
	curs.c = ' ';

	if (cli->cmdline.cur.cursor < cli->cmdline.cur.len) {
		curs.c = cli->cmdline.cur.buf[cli->cmdline.cur.cursor];
	}

	win_write_px(win, &p, &curs);
	win_clr_attr();

	p.x = 0;

	int32_t i;
	for (i = 0; i < win->rect.height; ++i) {
		if (*cli->cmdline.history.in[i]) {
			p.x = 0;
			++p.y;
			p.x = win_printf(win, &p, ":%s", cli->cmdline.history.in[i]);
			win_clrtoeol(win, &p);

			if (*cli->cmdline.history.out[i]) {
				p.x = 0;
				++p.y;
				p.x = win_printf(win, &p, "%s", cli->cmdline.history.out[i]);
				win_clrtoeol(win, &p);
			}
		} else {
			++p.y;
			win_clrtoeol(win, &p);
		}
	}
}

void
draw_infol(struct win *win, struct client *cli)
{
	if (cli->im == im_cmd) {
		draw_cli(win, cli);
		return;
	}

	struct point p = { 0, 0 };
	const char *act_tgt_nme;

	p.x += win_printf(win, &p, "redraws: (+%6d) | cmd: %5.5s%5.5s",
		cli->redrew_world, cli->num.buf, cli->cmd.buf);
	win_clrtoeol(win, &p);

	p.x = 0;
	p.y++;
	p.x = win_printf(win, &p,
		"view: (%4d, %4d) | cursor: (%4d, %4d) | rect: %dx%d",
		cli->view.x, cli->view.y, cli->cursor.x + cli->view.x,
		cli->cursor.y + cli->view.y,
		cli->next_act.range.width,
		cli->next_act.range.height
		);
	win_clrtoeol(win, &p);

	switch (cli->next_act.type) {
	case at_build:
		act_tgt_nme = gcfg.tiles[cli->next_act.tgt].name;
		break;
	default:
		act_tgt_nme = "";
		break;
	}
	p.x = 0;
	p.y++;
	p.x = win_printf(win, &p, "act: %s%c %s, %x",
		gcfg.actions[cli->next_act.type].name,
		*act_tgt_nme ? ',' : ' ', act_tgt_nme,
		cli->next_act.flags);
	win_clrtoeol(win, &p);

	/*
	   p.x = 0;
	   p.y++;
	   p.x = win_printf(win, &p, "motiv: %3d, ents : % 5ld, chunks:% 5ld ",
	        cli->sim->assigned_motivator,
	        hdarr_len(cli->world->ents), hdarr_len(cli->sim->w->chunks->hd));
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
print_ent_counts(struct win *win, struct client *cli, struct point *vp, struct point *cp)
{
	uint32_t ent_counts[ent_type_count] = { 0 };
	uint32_t tent_counts[ent_type_count] = { 0 };
	struct ent_count_ctx ctx = { ent_counts, tent_counts, cp };

	hdarr_for_each(&cli->world->ents, &ctx, ent_counter);

	vp->x = win_printf(win, vp, "w: %d / %d | @: %d / %d",
		ent_counts[et_resource_wood], tent_counts[et_resource_wood],
		ent_counts[et_worker], tent_counts[et_worker]);
	win_clrtoeol(win, vp);
}

void
draw_infor(struct win *win, struct client *cli)
{
	static struct point op = { 1, 1 };
	struct point vp = { 0, 0 };
	struct point cp = point_add(&cli->cursor, &cli->view);
	struct chunk *ck;

	if (cli->changed.ents) {
		print_ent_counts(win, cli, &vp, &cp);
	}

	vp.x = 0;
	++vp.y;

	if (!points_equal(&op, &cp)) {
		op = nearest_chunk(&cp);

		if ((ck = hdarr_get(&cli->world->chunks.hd, &op)) != NULL) {
			op = point_sub(&cp, &op);

			vp.x = win_printf(win, &vp, "tile: %-20.20s",
				gcfg.tiles[ck->tiles[op.x][op.y]].name);
			win_clrtoeol(win, &vp);
		}

		op = cp;
	}
}
