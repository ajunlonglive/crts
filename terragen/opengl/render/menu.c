#include "posix.h"

#include <math.h>
#include <string.h>
#include <time.h>

#include "shared/opengl/menu.h"
#include "shared/util/log.h"
#include "terragen/opengl/render/menu.h"
#include "terragen/opengl/worker.h"

enum sliders {
	slider_opacity = tg_opt_count,
	slider_count
};

static uint32_t slider_pad;
static struct menu_win_ctx main_win = { .w = 35, .title = "terragen opts" };
static struct menu_slider_ctx sliders[slider_count] = {
	[tg_radius]         = { .min = 0,   .max = 1                  },
	[tg_dim]            = { .min = 128, .max = 2048,  .step = 128 },
	[tg_points]         = { .min = 3,   .max = 10000, .step = 1   },
	[tg_mountains]      = { .min = 0,   .max = 1000,  .step = 1   },
	[tg_valleys]        = { .min = 0,   .max = 1000,  .step = 1   },
	[tg_fault_radius]   = { .min = 0,   .max = 20                 },
	[tg_fault_curve]    = { .min = 0,   .max = 10                 },
	[tg_height_mod]     = { .min = 0,   .max = 10                 },
	[tg_erosion_cycles] = { .min = 0,   .max = 1000,  .step = 1   },
	[tg_upscale]        = { .min = 1,   .max = 8,     .step = 1   },

	[slider_opacity]    = { .min = 0,   .max = 1                  },
};

void
render_terragen_menu_init(struct ui_ctx *ctx)
{
	slider_pad = 0;

	uint32_t i, len;
	for (i = tg_radius; i < tg_opt_count; ++i) {
		if ((len = strlen(terragen_opt_info[i].name)) > slider_pad) {
			slider_pad = len;
		}
	}

	main_win.h = tg_opt_count + 7;
}

void
render_terragen_menu(struct ui_ctx *ctx)
{
	menu_begin(&ctx->menu_ctx, ctx->mousex, ctx->mousey, ctx->mb_pressed);

	if (menu_win(&ctx->menu_ctx, &main_win)) {
		menu_str(&ctx->menu_ctx, "seed");
		ctx->menu_ctx.x += slider_pad - strlen("seed") - 2;

		menu_printf(&ctx->menu_ctx, "0x%08x", ctx->opts[tg_seed].u);
		ctx->menu_ctx.x += 1;

		if (menu_button(&ctx->menu_ctx, "randomize")) {
			struct timespec ts;
			clock_gettime(CLOCK_MONOTONIC, &ts);
			ctx->opts[tg_seed].u = ts.tv_nsec;

			start_genworld_worker(ctx);
		}

		menu_newline(&ctx->menu_ctx);
		menu_newline(&ctx->menu_ctx);

		uint32_t i;
		for (i = tg_radius; i < tg_opt_count; ++i) {

			menu_str(&ctx->menu_ctx, terragen_opt_info[i].name);
			ctx->menu_ctx.x += slider_pad - strlen(terragen_opt_info[i].name);

			switch (terragen_opt_info[i].t) {
			case dt_int: {
				menu_printf(&ctx->menu_ctx, "% 8d", ctx->opts[i].u);
				ctx->menu_ctx.x += 1;

				float v = ctx->opts[i].u;
				if (menu_slider(&ctx->menu_ctx, &sliders[i], &v)) {
					start_genworld_worker(ctx);
				}
				ctx->opts[i].u = v;
				break;
			}
			case dt_float: {
				menu_printf(&ctx->menu_ctx, "% 8.2f", ctx->opts[i].f);
				ctx->menu_ctx.x += 1;

				if (menu_slider(&ctx->menu_ctx, &sliders[i], &ctx->opts[i].f)) {
					start_genworld_worker(ctx);
				}
				break;
			}
			case dt_none:
				break;
			}

			menu_newline(&ctx->menu_ctx);
		}

		menu_newline(&ctx->menu_ctx);
		menu_str(&ctx->menu_ctx, "heightmap opacity");
		ctx->menu_ctx.x += 1;
		menu_slider(&ctx->menu_ctx, &sliders[slider_opacity], &ctx->heightmap_opacity);

		menu_newline(&ctx->menu_ctx);
		if (menu_button(&ctx->menu_ctx, "save")) {
			menu_str(&ctx->menu_ctx, " saving...");
			ctx->write_file = true;
		}
	}

	menu_render(&ctx->menu_ctx, &ctx->win);
}
