#include "posix.h"

#include <stddef.h>
#include <string.h>

#include "client/client.h"
#include "client/cmdline.h"
#include "client/ui/common.h"
#include "shared/util/log.h"

#ifdef NCURSES_UI
#include "client/ui/term/ui.h"
#endif

#ifdef OPENGL_UI

#include "client/ui/gl/cmdline.h"
#include "client/ui/gl/globals.h"
#include "client/ui/gl/input.h"
#include "client/ui/gl/render.h"
#include "client/ui/gl/ui.h"

#endif

void
ui_init(struct client_opts *opts, struct ui_ctx *ctx)
{
	ctx->enabled = opts->ui;

#ifdef OPENGL_UI
	if (ctx->enabled & ui_gl) {
		if (!(gl_ui_init(&ctx->gl))) {
			LOG_W(log_misc, "failed to initialize gl ui");
			ctx->enabled &= ~ui_gl;
		} else {
			LOG_I(log_misc, "initialized gl ui");
		}
	}
#endif

	/* enable term after gl to delay log redirection */
#ifdef NCURSES_UI
	if (ctx->enabled & ui_term) {
		if (!(term_ui_init(&ctx->term))) {
			ctx->enabled &= ~ui_term;
			LOG_W(log_misc, "failed to initialize term ui");
		} else {
			LOG_I(log_misc, "initialized term ui");
		}
	}
#endif

	if (!ctx->enabled) {
		LOG_I(log_misc, "using null ui ");
	}
}

void
ui_reset(struct client *cli)
{
/* #ifdef NCURSES_UI */
/* 	if (cli->ui_ctx->enabled & ui_term) { */
/* 		term_ui_reset(&cli->ui_ctx->term, cli); */
/* 	} */
/* #endif */

#ifdef OPENGL_UI
	if (cli->ui_ctx->enabled & ui_gl) {
		gl_ui_reset(&cli->ui_ctx->gl);
	}
#endif

}

void
ui_render(struct client *cli)
{
#ifdef NCURSES_UI
	if (cli->ui_ctx->enabled & ui_term) {
		term_ui_render(&cli->ui_ctx->term, cli);
	}
#endif

#ifdef OPENGL_UI
	if (cli->ui_ctx->enabled & ui_gl) {
		gl_ui_render(&cli->ui_ctx->gl, cli);
	}
#endif

	if (!(cli->ui_ctx->enabled & ui_gl)) {
		/* throttle rendering at 30 fps if we don't have to render gl too
		 */
		static struct timespec tick = { .tv_nsec = ((1.0f / 30.0f)) * 1000000000 };

		nanosleep(&tick, NULL);
	}
}

void
ui_handle_input(struct client *cli)
{
#ifdef NCURSES_UI
	if (cli->ui_ctx->enabled & ui_term) {
		term_ui_handle_input(&cli->ui_ctx->term, cli);
	}
#endif

#ifdef OPENGL_UI
	if (cli->ui_ctx->enabled & ui_gl) {
		gl_ui_handle_input(&cli->ui_ctx->gl, cli);
	}
#endif
}

struct rect *
ui_ref(struct client *cli)
{
#ifdef NCURSES_UI
	if (cli->ui_ctx->enabled & ui_term) {
		return term_ui_viewport(&cli->ui_ctx->term);
	}
#endif

#ifdef OPENGL_UI
	if (cli->ui_ctx->enabled & ui_gl) {
		return gl_ui_viewport(&cli->ui_ctx->gl);
	}
#endif

	return NULL;
}

vec3 *
ui_cam_pos(struct client *cli)
{
	static vec3 pos = { 0 };

#ifdef NCURSES_UI
	if (cli->ui_ctx->enabled & ui_term) {
		term_ui_handle_input(&cli->ui_ctx->term, cli);
		pos[0] = cli->view.x + cli->viewport.width / 2;
		pos[1] = 50;
		pos[2] = cli->view.y + cli->viewport.height / 2;

		return &pos;
	}
#endif

#ifdef OPENGL_UI
	if (cli->ui_ctx->enabled & ui_gl) {
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
	if (ctx->enabled & ui_term) {
		term_ui_deinit();
	}
#endif

#ifdef OPENGL_UI
	if (ctx->enabled & ui_gl) {
		gl_ui_deinit(&ctx->gl);
	}
#endif
}

enum cmd_result
ui_cmdline_hook(struct cmd_ctx *cmd, struct client *cli)
{
#ifdef NCURSES_UI
	if (cli->ui_ctx->enabled & ui_term) {
		/* TODO */
		/* return term_ui_cmdline_hook(cmd, ctx->term, cli); */
	}
#endif

#ifdef OPENGL_UI
	if (cli->ui_ctx->enabled & ui_gl) {
		return gl_ui_cmdline_hook(cmd, &cli->ui_ctx->gl, cli);
	}
#endif

	return cmdres_not_found;
}
