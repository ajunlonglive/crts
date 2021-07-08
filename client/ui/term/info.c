#include "posix.h"

#include "client/client.h"
#include "client/ui/term/graphics.h"
#include "client/ui/term/info.h"
#include "shared/constants/globals.h"
#include "shared/sim/world.h"
#include "shared/ui/term/window.h"
#include "shared/util/log.h"

static void
draw_cli(uint32_t win, struct client *cli)
{
	struct point p = { 0, 0 };

	p.x = term_printf(win, &p, ":%s", cli->cmdline.cur.buf);
	term_clrtoeol(win, &p);
	p.x = 1 + cli->cmdline.cur.cursor;

	struct term_pixel curs = graphics.cursor[ct_default].pix;
	curs.c = ' ';

	if (cli->cmdline.cur.cursor < cli->cmdline.cur.len) {
		curs.c = cli->cmdline.cur.buf[cli->cmdline.cur.cursor];
	}

	term_write_px(win, &p, &curs);
	term_clear_attr();

	p.x = 0;
	uint32_t i, h = term_win_rect(win)->height;
	for (i = 0; i < h; ++i) {
		if (*cli->cmdline.history.in[i]) {
			p.x = 0;
			++p.y;
			p.x = term_printf(win, &p, ":%s", cli->cmdline.history.in[i]);
			term_clrtoeol(win, &p);

			if (*cli->cmdline.history.out[i]) {
				p.x = 0;
				++p.y;
				p.x = term_printf(win, &p, "%s", cli->cmdline.history.out[i]);
				term_clrtoeol(win, &p);
			}
		} else {
			++p.y;
			term_clrtoeol(win, &p);
		}
	}
}

void
draw_infol(uint32_t win, struct client *cli)
{
	if (cli->im == im_cmd) {
		draw_cli(win, cli);
		return;
	}

	struct point p = { 0, 0 };

	float fps = 1.0 / (+cli->prof.client_tick.avg);

	p.x += term_printf(win, &p, "fps: %.1f (%6d) | sim: %.1f",
		fps,
		cli->redrew_world,
		cli->prof.server_fps
		);

	term_clrtoeol(win, &p);

	p.x = 0;
	p.y++;

	struct point _p = point_add(&cli->cursor, &cli->view),
		     q = nearest_chunk(&_p),
		     r = point_sub(&_p, &q);

	p.x = term_printf(win, &p, "curs:(%d, %d) ck:(%d, %d), rp:(%d, %d), idx:%d",
		_p.x, _p.y,
		q.x, q.y,
		r.x, r.y,
		r.y * 16 + r.x
		);
	term_clrtoeol(win, &p);
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
print_ent_counts(uint32_t win, struct client *cli, struct point *vp, struct point *cp)
{
	uint32_t ent_counts[ent_type_count] = { 0 };
	uint32_t tent_counts[ent_type_count] = { 0 };
	struct ent_count_ctx ctx = { ent_counts, tent_counts, cp };

	hdarr_for_each(&cli->world->ents, &ctx, ent_counter);

	vp->x = term_printf(win, vp, "w: %d / %d | @: %d / %d",
		ent_counts[et_resource_wood], tent_counts[et_resource_wood],
		ent_counts[et_worker], tent_counts[et_worker]);
	term_clrtoeol(win, vp);
}

void
draw_infor(uint32_t win, struct client *cli)
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

			vp.x = term_printf(win, &vp, "tile: %-20.20s",
				gcfg.tiles[ck->tiles[op.x][op.y]].name);
			term_clrtoeol(win, &vp);
		}

		op = cp;
	}
}
