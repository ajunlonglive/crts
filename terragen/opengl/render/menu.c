#include "posix.h"

#include <math.h>
#include <string.h>
#include <time.h>

#include "shared/opengl/menu.h"
#include "shared/util/log.h"
#include "terragen/opengl/render/menu.h"
#include "terragen/opengl/worker.h"

/* struct elem elements[] = { */
/* 	{ 0 }, */
/* 	{ 0 }, */
/* 	{ tg_seed,           e_button, .label = "shuf seed" }, */
/* 	{ tg_radius,         e_slider, 0.25, 0.5 }, */
/* 	{ tg_dim,            e_slider, 128, 2048, 128 }, */
/* 	{ tg_points,         e_slider, 0, 10000 }, */
/* 	{ 0 }, */
/* 	{ tg_mountains,      e_slider, 0, 100 }, */
/* 	{ tg_valleys,        e_slider, 0, 100 }, */
/* 	{ tg_fault_radius,   e_slider, 0, 100 }, */
/* 	{ tg_fault_curve,    e_slider, 0, 1 }, */
/* 	{ tg_height_mod,     e_slider, 0, 20 }, */
/* 	{ 0 }, */
/* 	{ tg_erosion_cycles, e_slider, 0, 10000 }, */
/* 	{ 0 }, */
/* 	{ tg_noise,          e_slider, 0, 1 }, */
/* 	{ 0 }, */
/* 	{ tg_upscale,        e_slider, 1, 4 }, */
/* 	{ -1 } */
/* }; */

void
render_terragen_menu_init(struct ui_ctx *ctx)
{
}

void
render_terragen_menu(struct ui_ctx *ctx)
{
	menu_begin(&ctx->menu_ctx, ctx->mousex, ctx->mousey, ctx->mb_pressed);

	static struct menu_win_ctx main_win = { .h = 10, .w = 30, .title = "terragen opts" };
	if (menu_win(&ctx->menu_ctx, &main_win)) {
		menu_printf(&ctx->menu_ctx, "seed: 0x%08x ", ctx->opts[tg_seed].u);
		if (menu_button(&ctx->menu_ctx, "randomize")) {
			struct timespec ts;
			clock_gettime(CLOCK_MONOTONIC, &ts);
			ctx->opts[tg_seed].u = ts.tv_nsec;

			start_genworld_worker(ctx);
		}

		menu_newline(&ctx->menu_ctx);
		menu_newline(&ctx->menu_ctx);

		menu_printf(&ctx->menu_ctx, "points: % 8d ", ctx->opts[tg_points].u);

		static struct menu_slider_ctx points_slider = { .min = 100, .max = 10000 };
		float v = ctx->opts[tg_points].u;
		if (menu_slider(&ctx->menu_ctx, &points_slider, &v)) {
			L("%d -> %f", ctx->opts[tg_points].u, v);
			start_genworld_worker(ctx);
		}
		ctx->opts[tg_points].u = v;
	}

	menu_render(&ctx->menu_ctx, &ctx->win);
}
