#include "posix.h"

#include <stddef.h>
#include <string.h>

#include "client/ui/common.h"
#include "shared/util/log.h"

#ifdef NCURSES_UI
#include "client/ui/ncurses/ui.h"
#endif

#ifdef OPENGL_UI
#include "client/ui/opengl/cmdline.h"
#include "client/ui/opengl/globals.h"
#include "client/ui/opengl/keymap_hook.h"
#include "client/ui/opengl/render.h"
#include "client/ui/opengl/ui.h"
#endif

void
ui_init(struct client_opts *opts, struct ui_ctx *ctx)
{
	ctx->enabled = opts->ui;

#ifdef OPENGL_UI
	if (ctx->enabled & ui_opengl) {
		if (!(opengl_ui_init(&ctx->opengl))) {
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
		if (!(ncurses_ui_init(&ctx->ncurses))) {
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
}

void
ui_render(struct client *cli)
{
#ifdef NCURSES_UI
	if (cli->ui_ctx->enabled & ui_ncurses) {
		ncurses_ui_render(&cli->ui_ctx->ncurses, cli);
	}
#endif

#ifdef OPENGL_UI
	if (cli->ui_ctx->enabled & ui_opengl) {
		opengl_ui_render(&cli->ui_ctx->opengl, cli);
	}
#endif
}

static void
fix_cursor(const struct rectangle *r, struct point *vu, struct point *cursor)
{
	int32_t diff;

	if ((diff = 0 - cursor->y) > 0 || (diff = (r->height - 1) - cursor->y) < 0) {
		vu->y -= diff;
		cursor->y += diff;
	}

	if ((diff = 0 - cursor->x) > 0 || (diff = (r->width - 1) - cursor->x) < 0) {
		vu->x -= diff;
		cursor->x += diff;
	}
}

void
ui_handle_input(struct client *cli)
{
#ifdef NCURSES_UI
	if (cli->ui_ctx->enabled & ui_ncurses) {
		ncurses_ui_handle_input(&cli->ui_ctx->ncurses, cli);
		cli->viewport = ncurses_ui_viewport(&cli->ui_ctx->ncurses);
	}
#endif

#ifdef OPENGL_UI
	if (cli->ui_ctx->enabled & ui_opengl) {
		opengl_ui_handle_input(&cli->ui_ctx->opengl, cli);
		cli->viewport = opengl_ui_viewport(&cli->ui_ctx->opengl);
	}
#endif

	fix_cursor(&cli->viewport, &cli->view, &cli->cursor);
}

vec3 *
ui_cam_pos(struct client *cli)
{
	static vec3 pos = { 0 };

#ifdef NCURSES_UI
	if (cli->ui_ctx->enabled & ui_ncurses) {
		ncurses_ui_handle_input(&cli->ui_ctx->ncurses, cli);
		pos[0] = cli->view.x + cli->viewport.width / 2;
		pos[1] = 50;
		pos[2] = cli->view.y + cli->viewport.height / 2;

		return &pos;
	}
#endif

#ifdef OPENGL_UI
	if (cli->ui_ctx->enabled & ui_opengl) {
		memcpy(pos, cam.pos, sizeof(float) * 3);

		return &pos;
	}
#endif

	return &pos;
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
		opengl_ui_deinit(&ctx->opengl);
	}
#endif
}

enum cmd_result
ui_cmdline_hook(struct cmd_ctx *cmd, struct client *cli)
{
#ifdef NCURSES_UI
	if (cli->ui_ctx->enabled & ui_ncurses) {
		/* TODO */
		/* return ncurses_ui_cmdline_hook(cmd, ctx->ncurses, cli); */
	}
#endif

#ifdef OPENGL_UI
	if (cli->ui_ctx->enabled & ui_opengl) {
		return opengl_ui_cmdline_hook(cmd, &cli->ui_ctx->opengl, cli);
	}
#endif

	return cmdres_not_found;
}


enum keymap_hook_result
ui_keymap_hook(struct ui_ctx *ctx, struct keymap *km, char *err, const char *sec, const char *k, const char *v, uint32_t line)
{
#ifdef NCURSES_UI
	if (ctx->enabled & ui_ncurses) {
		/* TODO */
	}
#endif

#ifdef OPENGL_UI
	if (ctx->enabled & ui_opengl) {
		return opengl_ui_keymap_hook(&ctx->opengl, err, sec, k, v, line);
	}
#endif

	return khr_unmatched;
}
