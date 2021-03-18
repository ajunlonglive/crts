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

	/* menu_str(&ctx->menu_ctx, "test!!"); */
	/* menu_str(&ctx->menu_ctx, "quest!!"); */
	static float menux = 0.0, menuy = 0.0;

	menu_win(&ctx->menu_ctx, &menux, &menuy, 10, 10);

	if (menu_button(&ctx->menu_ctx, "test!!")) {
		menu_str(&ctx->menu_ctx, "you clicked it!");
	}
	menu_str(&ctx->menu_ctx, "some info...");

	menu_render(&ctx->menu_ctx, &ctx->win);
}
