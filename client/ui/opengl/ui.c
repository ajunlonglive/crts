#include "posix.h"

#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "client/ui/opengl/globals.h"
#include "client/ui/opengl/input.h"
#include "client/ui/opengl/loaders/color_cfg.h"
#include "client/ui/opengl/render.h"
#include "client/ui/opengl/render/text.h"
#include "client/ui/opengl/ui.h"
#include "client/ui/opengl/window.h"
#include "shared/util/log.h"

static void
resize_callback(struct GLFWwindow *win, int width, int height)
{
	struct opengl_ui_ctx *ctx = glfwGetWindowUserPointer(win);

	glViewport(0, 0, width, height);

	gen_perspective_mat4(FOV, (float)width / (float)height, NEAR, 1000.0,
		ctx->mproj);

	ctx->width = width;
	ctx->height = height;

	ctx->resized = true;
}

struct opengl_ui_ctx *
opengl_ui_init(struct c_opts *opts)
{
	int x, y;
	struct opengl_ui_ctx *ctx = calloc(1, sizeof(struct opengl_ui_ctx));

	if (!(ctx->window = init_window())) {
		goto free_exit;
	}

	glfwSetWindowUserPointer(ctx->window, ctx);

	/* load color config */
	color_cfg();

	/* Set callbacks */
	set_input_callbacks(ctx->window);
	glfwSetFramebufferSizeCallback(ctx->window, resize_callback);

	/* setup rendering */
	if (!opengl_ui_render_setup(opts)) {
		goto free_exit;
	}

	glClearColor(colors.tile_fg[tile_deep_water][0],
		colors.tile_fg[tile_deep_water][1],
		colors.tile_fg[tile_deep_water][2], 1.0);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);

	glfwGetWindowSize(ctx->window, &x, &y);
#ifdef __APPLE__
	/* HACK macOS has a black screen before being resized */

	glfwSwapBuffers(ctx->window);

	x += 1; y += 1;
	glfwSetWindowSize(ctx->window, x, y);
#endif
	ctx->width = x;
	ctx->height = y;

	return ctx;
free_exit:
	opengl_ui_deinit(ctx);
	return NULL;
}

void
opengl_ui_handle_input(struct opengl_ui_ctx *ctx, struct keymap **km,
	struct hiface *hf)
{
	glfwPollEvents();

	struct camera ocam = cam;

	handle_held_keys(ctx, hf, km);
	handle_gl_mouse(ctx, hf);

	if (memcmp(&ocam, &cam, sizeof(struct camera)) != 0) {
		ocam = cam;
		cam.changed = true;
	}

	if (glfwWindowShouldClose(ctx->window)) {
		hf->sim->run = false;
	} else if (!hf->sim->run) {
		glfwSetWindowShouldClose(ctx->window, 1);
	}
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
	free(ctx);
	glfwTerminate();
}
