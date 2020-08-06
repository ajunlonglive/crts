#include "posix.h"

#include <time.h>

#include "genworld/gl.h"
#include "shared/opengl/render/text.h"
#include "shared/opengl/window.h"
#include "shared/util/log.h"
#include "shared/util/time.h"

#define TICK NS_IN_S / 30

struct ui_ctx {
	struct GLFWwindow *glfw_win;
	struct gl_win win;
};

static void
resize_callback(struct GLFWwindow *win, int width, int height)
{
	struct ui_ctx *ctx = glfwGetWindowUserPointer(win);

	ctx->win.width = width;
	ctx->win.height = height;
	ctx->win.resized = true;
}

static void
genworld_interactive_setup(struct ui_ctx *ctx)
{
	if (!(ctx->glfw_win = init_window())) {
		return;
	}

	glfwSetWindowUserPointer(ctx->glfw_win, &ctx);
	glfwSetFramebufferSizeCallback(ctx->glfw_win, resize_callback);
	/* glfwSetKeyCallback(ctx->glfw_win, key_callback); */
	/* glfwSetCursorPosCallback(ctx->glfw_win, mouse_callback); */
	/* glfwSetScrollCallback(ctx->glfw_win, scroll_callback); */
	/* glfwSetMouseButtonCallback(ctx->glfw_win, mouse_button_callback); */

	glfwGetWindowSize(ctx->glfw_win, (int *)&ctx->win.width, (int *)&ctx->win.height);
	ctx->win.resized = true;

	glClearColor(1, 1, 1, 1.0);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glEnable(GL_FRAMEBUFFER_SRGB);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);

	if (!render_text_setup(15.0)) {
		return;
	}
}

static void
handle_input(struct ui_ctx *ctx)
{

}

void
genworld_interactive(struct worldgen_opts *opts)
{
	struct ui_ctx ctx = { 0 };
	long slept_ns = 0;
	struct timespec tick_st;
	clock_gettime(CLOCK_MONOTONIC, &tick_st);

	genworld_interactive_setup(&ctx);

	while (1) {
		glViewport(0, 0, ctx.win.width, ctx.win.height);
		glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
		glfwPollEvents();

		handle_input(&ctx);

		render_text_clear();
		float sx, sy;
		screen_coords_to_text_coords(0, 0, &sx, &sy);
		gl_printf(sx, sy, ta_left, "hello worldx");
		render_text_commit();
		render_text(&ctx.win);

		glfwSwapBuffers(ctx.glfw_win);

		slept_ns = sleep_remaining(&tick_st, TICK, slept_ns);

		ctx.win.resized = false;
	}

	glfwTerminate();
}
