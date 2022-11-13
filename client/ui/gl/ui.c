#include "posix.h"

#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "client/ui/gl/colors.h"
#include "client/ui/gl/globals.h"
#include "client/ui/gl/input.h"
#include "client/ui/gl/render.h"
#include "client/ui/gl/ui.h"
#include "shared/util/log.h"

void
gl_ui_reset(struct gl_ui_ctx *ctx)
{
	ctx->time.sun_theta_tgt = 6.872234; /* 10:45 */
	ctx->time.sun_theta = 0;
	ctx->view_was_initialized = false;

	ctx->win = gl_win_init();
	set_input_callbacks(ctx);
	gl_win_set_cursor_display(false);
	glClearColor(colors.sky[0], colors.sky[1], colors.sky[2], 1.0);
	timer_lap(&ctx->timer);
}

bool
gl_ui_init(struct gl_ui_ctx *ctx)
{
	ctx->time.sun_theta_tgt = 6.872234; /* 10:45 */
	timer_lap(&ctx->timer);

	if (!(ctx->win = gl_win_init())) {
		goto free_exit;
	}

	set_input_callbacks(ctx);
	register_input_cfg_data();

#if 0
	if (!parse_opengl_cfg(&ctx->opts)) {
		goto free_exit;
	}
#endif

	gl_win_set_cursor_display(false);

	if (!gl_ui_render_setup(ctx)) {
		goto free_exit;
	}

	glClearColor(colors.sky[0], colors.sky[1], colors.sky[2], 1.0);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

#ifndef NDEBUG
	darr_init(&ctx->debug_hl_points, sizeof(struct point));
#endif

	return true;
free_exit:
	gl_ui_deinit(ctx);
	return false;
}

struct rect *
gl_ui_viewport(struct gl_ui_ctx *nc)
{
	return NULL; //&nc->ref;
}

void
gl_ui_deinit(struct gl_ui_ctx *ctx)
{
	gl_ui_render_teardown();
	gl_win_terminate();
}
