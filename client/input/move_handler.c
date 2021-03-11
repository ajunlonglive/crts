#include "posix.h"

#include <stdlib.h>

#include "client/client.h"
#include "client/input/handler.h"
#include "client/input/helpers.h"
#include "client/input/move_handler.h"
#include "shared/constants/globals.h"
#include "shared/sim/ent.h"
#include "shared/util/log.h"

#define DEF_MOVE_AMNT 1

void
center(struct client *d)
{
	d->view.x = 0;
	d->view.y = 0;
}

void
center_cursor(struct client *cli)
{
	cli->view.x += cli->cursor.x - cli->viewport.width / 2;
	cli->view.y += cli->cursor.y - cli->viewport.height / 2;
	cli->cursor.x = cli->viewport.width / 2;
	cli->cursor.y = cli->viewport.height / 2;
}

void *cursor, *view, *up, *down, *left, *right;

#define gen_move(a, b, body) \
	void \
	a ## _ ## b(struct client *cli) { \
		long num = client_get_num(cli, DEF_MOVE_AMNT); \
		body; \
	}

gen_move(cursor, up,    cli->cursor.y -= num)
gen_move(cursor, down,  cli->cursor.y += num)
gen_move(cursor, left,  cli->cursor.x -= num)
gen_move(cursor, right, cli->cursor.x += num)
gen_move(view,   up,    cli->view.y -= num)
gen_move(view,   down,  cli->view.y += num)
gen_move(view,   left,  cli->view.x -= num)
gen_move(view,   right, cli->view.x += num)

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
find(struct client *d)
{
	enum ent_type tgt = client_get_num(d, et_worker);

	tgt %= ent_type_count;

	struct point p = point_add(&d->view, &d->cursor);

	struct find_ctx ctx = { tgt, &p, NULL, UINT32_MAX };
	hdarr_for_each(&d->world->ents, &ctx, find_iterator);

	if (ctx.res) {
		d->view = ctx.res->pos;
		d->cursor.x = 0;
		d->cursor.y = 0;
		center_cursor(d);
	}
}
