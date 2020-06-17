#include <stdlib.h>

#include "client/ui/common.h"
#include "shared/util/log.h"

#ifdef NCURSES_UI
#include "client/ui/ncurses/ui.h"
#endif

#ifdef OPENGL_UI
#include "client/ui/opengl/render.h"
#include "client/ui/opengl/ui.h"
#endif

struct ui_ctx {
	struct ncurses_ui_ctx *ncurses;
	struct opengl_ui_ctx *opengl;
	uint8_t enabled;
};

struct ui_ctx *
ui_init(struct c_opts *opts)
{
	struct ui_ctx *ctx = calloc(1, sizeof(struct ui_ctx));

	ctx->enabled = opts->ui;

#ifdef OPENGL_UI
	if (ctx->enabled & ui_opengl) {
		if (!(ctx->opengl = opengl_ui_init())) {
			LOG_W("failed to initialize opengl ui");
			ctx->enabled &= ~ui_opengl;
		} else {
			LOG_I("initialized opengl ui");
		}
	}
#endif

	/* enable ncurses after opengl to delay log redirection */
#ifdef NCURSES_UI
	if (ctx->enabled & ui_ncurses) {
		if (!(ctx->ncurses = ncurses_ui_init(opts->logfile))) {
			ctx->enabled &= ~ui_ncurses;
			LOG_W("failed to initialize ncurses ui");
		} else {
			LOG_I("initialized ncurses ui");
		}
	}
#endif

	if (!ctx->enabled) {
		LOG_I("using null ui ");
	}

	return ctx;
}

void
ui_render(struct ui_ctx *ctx, struct hiface *hf)
{
#ifdef NCURSES_UI
	if (ctx->enabled & ui_ncurses) {
		ncurses_ui_render(ctx->ncurses, hf);
	}
#endif

#ifdef OPENGL_UI
	if (ctx->enabled & ui_opengl) {
		opengl_ui_render(ctx->opengl, hf);
	}
#endif
}

void
ui_handle_input(struct ui_ctx *ctx, struct keymap **km, struct hiface *hf)
{
#ifdef NCURSES_UI
	if (ctx->enabled & ui_ncurses) {
		ncurses_ui_handle_input(km, hf);
	}
#endif

#ifdef OPENGL_UI
	if (ctx->enabled & ui_opengl) {
		opengl_ui_handle_input(ctx->opengl, km, hf);
	}
#endif
}

struct rectangle
ui_viewport(struct ui_ctx *ctx)
{
#ifdef NCURSES_UI
	if (ctx->enabled & ui_ncurses) {
		return ncurses_ui_viewport(ctx->ncurses);
	}
#endif

#ifdef OPENGL_UI
	if (ctx->enabled & ui_opengl) {
		return opengl_ui_viewport(ctx->opengl);
	}
#endif

	struct rectangle r = { 0 };
	return r;
}

void
ui_deinit(struct ui_ctx *ctx)
{
#ifdef NCURSES_UI
	if (ctx->enabled & ui_ncurses) {
		ncurses_ui_deinit();
	}
#endif

#ifdef OPENGL_UI
	if (ctx->enabled & ui_opengl) {
		opengl_ui_deinit(ctx->opengl);
	}
#endif
}
