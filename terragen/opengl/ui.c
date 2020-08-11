#include "posix.h"

#include <time.h>

#include "shared/opengl/render/text.h"
#include "shared/opengl/window.h"
#include "shared/util/log.h"
#include "shared/util/time.h"
#include "terragen/opengl/render/menu.h"
#include "terragen/opengl/render/mesh.h"
#include "terragen/opengl/ui.h"
#include "terragen/opengl/worker.h"

#define TICK NS_IN_S / 30

static void
resize_callback(struct GLFWwindow *win, int width, int height)
{
	struct ui_ctx *ctx = glfwGetWindowUserPointer(win);

	ctx->win.width = width;
	ctx->win.height = height;
	ctx->win.resized = true;
}

static void
mouse_callback(struct GLFWwindow *win, double x, double y)
{
	struct ui_ctx *ctx = glfwGetWindowUserPointer(win);

	ctx->mousex = x;
	ctx->mousey = y;
}

static void
mouse_button_callback(GLFWwindow* win, int button, int action, int _mods)
{
	struct ui_ctx *ctx = glfwGetWindowUserPointer(win);

	++button;

	if (action == GLFW_PRESS) {
		ctx->mb |= button;
	} else {
		ctx->mb &= ~button;
	}
}

static bool
genworld_interactive_setup(struct ui_ctx *ctx)
{
	ctx->text_scale = 15.0f;

	if (!(ctx->glfw_win = init_window())) {
		return false;
	} else if (!render_text_setup(ctx->text_scale)) {
		return false;
	} else if (!render_mesh_setup(ctx)) {
		return false;
	}


	glfwSetWindowUserPointer(ctx->glfw_win, ctx);
	glfwSetFramebufferSizeCallback(ctx->glfw_win, resize_callback);
	/* glfwSetKeyCallback(ctx->glfw_win, key_callback); */
	glfwSetCursorPosCallback(ctx->glfw_win, mouse_callback);
	/* glfwSetScrollCallback(ctx->glfw_win, scroll_callback); */
	glfwSetMouseButtonCallback(ctx->glfw_win, mouse_button_callback);

	glfwGetWindowSize(ctx->glfw_win, (int *)&ctx->win.width, (int *)&ctx->win.height);
	ctx->win.resized = true;

	glClearColor(1, 1, 1, 1.0);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glEnable(GL_FRAMEBUFFER_SRGB);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);

	return true;
}

void
genworld_interactive(struct terragen_opts *opts)
{
	struct ui_ctx ctx = { 0 };
	long slept_ns = 0;
	struct timespec tick_st;
	clock_gettime(CLOCK_MONOTONIC, &tick_st);

	if (!genworld_interactive_setup(&ctx)) {
		return;
	}

	ctx.opts = opts;
	init_genworld_worker();
	start_genworld_worker(&ctx);

	while (!glfwWindowShouldClose(ctx.glfw_win)) {
		glViewport(0, 0, ctx.win.width, ctx.win.height);
		glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

		glfwPollEvents();

		render_mesh_setup_frame(&ctx);
		render_mesh(&ctx);

		render_text_clear();
		render_menu(&ctx);
		render_text_commit();
		render_text(&ctx.win);

		glfwSwapBuffers(ctx.glfw_win);

		slept_ns = sleep_remaining(&tick_st, TICK, slept_ns);

		ctx.win.resized = false;
	}

	glfwTerminate();
}
