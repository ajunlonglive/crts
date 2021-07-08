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

bool
gl_ui_init(struct gl_ui_ctx *ctx)
{
	ctx->time.sun_theta_tgt = 6.872234; /* 10:45 */

	if (!(ctx->win = gl_win_init(ctx))) {
		goto free_exit;
	}

	set_input_callbacks(ctx);

	if (!parse_opengl_cfg(&ctx->opts)) {
		goto free_exit;
	}

	gl_win_set_cursor_display(false);

	if (!gl_ui_render_setup(ctx)) {
		goto free_exit;
	}

	glClearColor(colors.sky[0], colors.sky[1], colors.sky[2], 1.0);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glEnable(GL_FRAMEBUFFER_SRGB);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);

#ifndef NDEBUG
	darr_init(&ctx->debug_hl_points, sizeof(struct point));
#endif

	return true;
free_exit:
	gl_ui_deinit(ctx);
	return false;
}

const struct rectangle *
gl_ui_viewport(struct gl_ui_ctx *nc)
{
	return &nc->ref;
}

void
gl_ui_deinit(struct gl_ui_ctx *ctx)
{
	gl_ui_render_teardown();
	gl_win_terminate();
}
