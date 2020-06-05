#include "posix.h"

#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "client/ui/opengl/color_cfg.h"
#include "client/ui/opengl/globals.h"
#include "client/ui/opengl/hud.h"
#include "client/ui/opengl/input.h"
#include "client/ui/opengl/render_world.h"
#include "client/ui/opengl/text.h"
#include "client/ui/opengl/ui.h"
#include "client/ui/opengl/winutil.h"
#include "shared/util/log.h"

/* Needed for resize_callback */
static struct opengl_ui_ctx *global_ctx;

static void
resize_callback(struct GLFWwindow *win, int width, int height)
{
	glViewport(0, 0, width, height);

	gen_perspective_mat4(FOV, (float)width / (float)height, NEAR, 1000.0,
		global_ctx->mproj);

	update_world_viewport(global_ctx->mproj);

	update_text_viewport(width, height);

	global_ctx->width = width;
	global_ctx->height = height;

	global_ctx->resized = true;
}

struct opengl_ui_ctx *
opengl_ui_init(char *graphics_path)
{
	int x, y;
	struct opengl_ui_ctx *ctx = calloc(1, sizeof(struct opengl_ui_ctx));
	global_ctx = ctx;

	if (!(ctx->window = init_window())) {
		goto free_exit;
	}

	glfwSetWindowUserPointer(ctx->window, ctx);

	/* load color config */
	color_cfg(graphics_path);

	/* Set callbacks */
	set_input_callbacks(ctx->window);
	glfwSetFramebufferSizeCallback(ctx->window, resize_callback);

	/* setup programs */
	render_world_setup();
	text_init();

	glClearColor(colors.tile[tile_deep_water][0],
		colors.tile[tile_deep_water][1], colors.tile[tile_deep_water][2], 1.0);

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
	global_ctx->width = x;
	global_ctx->height = y;

	return ctx;
free_exit:
	opengl_ui_deinit(ctx);
	return NULL;
}

void
opengl_ui_render(struct opengl_ui_ctx *ctx, struct hiface *hf)
{
	static double last_start = 0.0;

	double start = glfwGetTime(), stop;

	ctx->pulse += start - last_start;
	if (ctx->pulse > 2 * PI) {
		ctx->pulse -= 2 * PI;
	}

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	/* render hud before world, since it can affect selection state */
	render_hud(ctx, hf);

	if (cam.unlocked) {
		render_debug_hud(ctx);
	}

	render_world(ctx, hf);

	ctx->prof.setup = glfwGetTime() - start;

	glfwSwapBuffers(ctx->window);

	stop = glfwGetTime();
	ctx->prof.render = stop - ctx->prof.setup - start;
	ctx->prof.ftime = stop - start;
	last_start = start;

	ctx->resized = false;
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
	render_world_teardown();
	free(ctx);
	glfwTerminate();
}
