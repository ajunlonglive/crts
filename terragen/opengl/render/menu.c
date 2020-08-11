#include "posix.h"

#include <time.h>

#include "shared/opengl/render/text.h"
#include "terragen/opengl/render/menu.h"
#include "terragen/opengl/worker.h"

static void
shuffle_seed(struct ui_ctx *ctx)
{
	struct timespec ts;
	clock_gettime(CLOCK_MONOTONIC, &ts);
	ctx->opts->seed = ts.tv_nsec;
	start_genworld_worker(ctx);
}

static void
inc_points(struct ui_ctx *ctx)
{
	ctx->opts->points *= 2;
	start_genworld_worker(ctx);
}
static void
dec_points(struct ui_ctx *ctx)
{
	ctx->opts->points /= 2;
	start_genworld_worker(ctx);
}

static void
inc_faults(struct ui_ctx *ctx)
{
	ctx->opts->faults *= 2;
	start_genworld_worker(ctx);
}

static void
dec_faults(struct ui_ctx *ctx)
{
	ctx->opts->faults /= 2;
	start_genworld_worker(ctx);
}

typedef void ((*menu_cb_func)(struct ui_ctx *ctx));

struct menu_cb_ctx {
	struct ui_ctx *ctx;
	menu_cb_func func;
};

static void
render_menu_cb(void *_ctx, bool hvr, float x, float y, const char *str)
{
	struct menu_cb_ctx *ctx = _ctx;
	vec4 clr = { 0, 0, 0, 1 };
	if (hvr) {
		if (ctx->ctx->mb & 1) {
			ctx->ctx->mb &= ~1;
			ctx->func(ctx->ctx);
			clr[2] = 1.0f;
		} else {
			clr[1] = 1.0f;
		}
	} else {
		clr[0] = clr[1] = clr[2] = 1.0f;
	};

	gl_write_string(x, y, 1.0, clr, str);
}

void
render_menu(struct ui_ctx *ctx)
{
	float sx, sy;
	struct menu_cb_ctx mctx = { ctx };
	struct pointf mouse = { ctx->mousex / ctx->text_scale,
				(ctx->win.height - ctx->mousey) / ctx->text_scale };

	mctx.func = shuffle_seed;
	screen_coords_to_text_coords(0, -1, &sx, &sy);
	gl_printf(sx, sy, ta_left, "seed: ");
	gl_mprintf(6, -1, ta_left, &mouse, &mctx, render_menu_cb, "%10d", ctx->ctx.opts.seed);

	screen_coords_to_text_coords(0, -2, &sx, &sy);
	gl_printf(sx, sy, ta_left, "pts:  %8d", ctx->ctx.opts.points);

	mctx.func = dec_points;
	gl_mprintf(14, -2, ta_left, &mouse, &mctx, render_menu_cb, "/");

	mctx.func = inc_points;
	gl_mprintf(15, -2, ta_left, &mouse, &mctx, render_menu_cb, "*");

	screen_coords_to_text_coords(0, -3, &sx, &sy);
	gl_printf(sx, sy, ta_left, "faults:  %5d", ctx->ctx.opts.faults);

	mctx.func = dec_faults;
	gl_mprintf(14, -3, ta_left, &mouse, &mctx, render_menu_cb, "/");

	mctx.func = inc_faults;
	gl_mprintf(15, -3, ta_left, &mouse, &mctx, render_menu_cb, "*");
}

