#include <curses.h>

#include "client/display/info.h"
#include "client/display/window.h"
#include "client/hiface.h"
#include "shared/sim/world.h"
#include "shared/constants/globals.h"

void
draw_infol(struct win *win, struct hiface *hif)
{
	struct point p = { 0, 0 };

	win_printf(win, &p, "redraws: (+%6d) | ping: (+%6uf)", hif->redrew_world,
		hif->server_timeout);
	p.y++;
	win_printf(win, &p, "view: (%4d, %4d) | cursor: (%4d, %4d)",
		hif->view.x, hif->view.y,
		hif->cursor.x + hif->view.x,
		hif->cursor.y + hif->view.y);
	p.y++;
	win_printf(win, &p, "act: %s | cmd: %5.5s%5.5s",
		gcfg.actions[hif->next_act.type].name, hif->num.buf, hif->cmd.buf);
	p.y++;
	win_printf(win, &p, "motiv: %3d, ents : % 5ld, chunks:% 5ld ",
		hif->sim->assigned_motivator,
		hdarr_len(hif->sim->w->ents), hdarr_len(hif->sim->w->chunks->hd));
}

struct ent_count_ctx {
	uint32_t *ec;
	const struct point *p;
};

static enum iteration_result
ent_counter(void *_ctx, void *_e)
{
	struct ent_count_ctx *ctx = _ctx;
	struct ent *e = _e;

	if (points_equal(&e->pos, ctx->p)) {
		++(ctx->ec[e->type]);
	}

	return ir_cont;
}

void
draw_infor(struct win *win, struct hiface *hif)
{
	if (!hif->sim->changed.ents) {
		return;
	}

	struct point p = point_add(&hif->cursor, &hif->view);
	uint32_t ent_counts[ent_type_count] = { 0 };
	struct ent_count_ctx ctx = { ent_counts, &p };

	hdarr_for_each(hif->sim->w->ents, &ctx, ent_counter);

	p.x = p.y = 0;
	win_printf(win, &p, "w: %d | @: %d", ent_counts[et_resource_wood], ent_counts[et_worker]);
	clrtoeol();
}
