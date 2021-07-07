#include "posix.h"

#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "client/ui/opengl/colors.h"
#include "client/ui/opengl/globals.h"
#include "client/ui/opengl/input.h"
#include "client/ui/opengl/render.h"
#include "client/ui/opengl/ui.h"
#include "shared/opengl/window.h"
#include "shared/util/log.h"

bool
opengl_ui_init(struct opengl_ui_ctx *ctx)
{
	ctx->time.sun_theta_tgt = 6.872234; /* 10:45 */

	if (!(ctx->win = win_init(ctx))) {
		goto free_exit;
	}

	set_input_callbacks(ctx);

	/* load opengl cfg */
	if (!parse_opengl_cfg(&ctx->opts)) {
		goto free_exit;
	}

	/* set input mode */
	win_set_cursor_display(false);

	/* setup rendering */
	if (!opengl_ui_render_setup(ctx)) {
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
	opengl_ui_deinit(ctx);
	return false;
}

struct rectangle
opengl_ui_viewport(struct opengl_ui_ctx *nc)
{
	return nc->ref;
}

void
opengl_ui_deinit(struct opengl_ui_ctx *ctx)
{
	opengl_ui_render_teardown();
	win_terminate();
}
