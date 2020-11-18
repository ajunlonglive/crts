#include "posix.h"

#include <stdlib.h>

#include "client/hiface.h"
#include "client/input/handler.h"
#include "client/input/move_handler.h"
#include "shared/constants/globals.h"
#include "shared/sim/ent.h"
#include "shared/util/log.h"

#define DEF_MOVE_AMNT 1

void
center(struct hiface *d)
{
	if (d->keymap_describe) {
		hf_describe(d, kmc_nav, "return view to 0, 0");
		return;
	}

	d->view.x = 0;
	d->view.y = 0;
}

void
center_cursor(struct hiface *d)
{
	if (d->keymap_describe) {
		hf_describe(d, kmc_nav, "center cursor");
		return;
	}

	d->center_cursor = true;
}

void *cursor, *view, *up, *down, *left, *right;

#define gen_move(a, b, body) \
	void \
	a ## _ ## b(struct hiface *hf) { \
		long num = hiface_get_num(hf, DEF_MOVE_AMNT); \
		if (hf->keymap_describe) { \
			hf_describe(hf, kmc_nav, #a " %-5.5s  %d", #b, num); \
			return; \
		} \
		body; \
	}

gen_move(cursor, up,    hf->cursor.y -= num)
gen_move(cursor, down,  hf->cursor.y += num)
gen_move(cursor, left,  hf->cursor.x -= num)
gen_move(cursor, right, hf->cursor.x += num)
gen_move(view,   up,    hf->view.y -= num)
gen_move(view,   down,  hf->view.y += num)
gen_move(view,   left,  hf->view.x -= num)
gen_move(view,   right, hf->view.x += num)

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

	if (d->keymap_describe) {
		hf_describe(d, kmc_nav, "find nearest %s", gcfg.ents[tgt].name);
		return;
	}
	struct point p = point_add(&d->view, &d->cursor);

	struct find_ctx ctx = { tgt, &p, NULL, UINT32_MAX };
	hdarr_for_each(&d->sim->w->ents, &ctx, find_iterator);

	if (ctx.res) {
		d->view = ctx.res->pos;
		d->cursor.x = 0;
		d->cursor.y = 0;
		d->center_cursor = true;
	}
}
