#include <stdlib.h>

#include "client/hiface.h"
#include "client/input/handler.h"
#include "client/input/move_handler.h"
#include "shared/sim/ent.h"
#include "shared/util/log.h"

#define DEF_MOVE_AMNT 1

void
center(struct hiface *d)
{
	d->view.x = 0;
	d->view.y = 0;
}

void
center_cursor(struct hiface *d)
{
	d->center_cursor = true;
}

void
cursor_up(struct hiface *d)
{
	d->cursor.y -= hiface_get_num(d, DEF_MOVE_AMNT);
}

void
cursor_down(struct hiface *d)
{
	d->cursor.y += hiface_get_num(d, DEF_MOVE_AMNT);
}

void
cursor_left(struct hiface *d)
{
	d->cursor.x -= hiface_get_num(d, DEF_MOVE_AMNT);
}

void
cursor_right(struct hiface *d)
{
	d->cursor.x += hiface_get_num(d, DEF_MOVE_AMNT);
}

void
view_up(struct hiface *d)
{
	d->view.y -= hiface_get_num(d, DEF_MOVE_AMNT);
}

void
view_down(struct hiface *d)
{
	d->view.y += hiface_get_num(d, DEF_MOVE_AMNT);
}

void
view_left(struct hiface *d)
{
	d->view.x -= hiface_get_num(d, DEF_MOVE_AMNT);
}

void
view_right(struct hiface *d)
{
	d->view.x += hiface_get_num(d, DEF_MOVE_AMNT);
}

void
end_simulation(struct hiface *d)
{
	d->sim->run = 0;
}

void
set_input_mode_select(struct hiface *d)
{
	d->im = im_select;
}

void
set_input_mode_normal(struct hiface *d)
{
	d->im = im_normal;
}

struct find_ctx {
	enum ent_type t;
	struct point *p;
	struct ent *res;
	uint32_t mindist;
};

static enum iteration_result
find_iterator(void *_ctx, void *_e)
{
	struct ent *e = _e;
	struct find_ctx *ctx = _ctx;
	uint32_t dist;

	if (ctx->t == e->type
	    && (dist = square_dist(ctx->p, &e->pos)) < ctx->mindist) {
		ctx->mindist = dist;
		ctx->res = e;
	}

	return ir_cont;
}

void
find(struct hiface *d)
{
	enum ent_type tgt = hiface_get_num(d, et_worker);

	tgt %= ent_type_count;

	struct find_ctx ctx = { tgt, &d->cursor, NULL, UINT32_MAX };
	hdarr_for_each(d->sim->w->ents, &ctx, find_iterator);

	if (ctx.res) {
		d->view = ctx.res->pos;
		d->cursor.x = 0;
		d->cursor.y = 0;
		d->center_cursor = true;
	}
}
