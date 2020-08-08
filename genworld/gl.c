#include "posix.h"

#include <time.h>

#include "genworld/gl.h"
#include "genworld/worker.h"
#include "genworld/render_terrain.h"
#include "shared/opengl/render/text.h"
#include "shared/opengl/window.h"
#include "shared/util/log.h"
#include "shared/util/time.h"

#define TICK NS_IN_S / 30
#define SCALE 15.0f

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
	if (!(ctx->glfw_win = init_window())) {
		return false;
	} else if (!render_text_setup(SCALE)) {
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

static void
shuffle_seed(struct ui_ctx *ctx)
{
	struct timespec ts;
	clock_gettime(CLOCK_MONOTONIC, &ts);
	ctx->opts->seed = ts.tv_nsec;
	start_genworld_worker((struct gen_terrain_ctx *)&ctx->gt, ctx->opts, gts_triangulate);
}

static void
inc_points(struct ui_ctx *ctx)
{
	ctx->opts->points *= 2;
	start_genworld_worker((struct gen_terrain_ctx *)&ctx->gt, ctx->opts, gts_triangulate);
}
static void
dec_points(struct ui_ctx *ctx)
{
	ctx->opts->points /= 2;
	start_genworld_worker((struct gen_terrain_ctx *)&ctx->gt, ctx->opts, gts_triangulate);
}

static void
inc_faults(struct ui_ctx *ctx)
{
	ctx->opts->faults *= 2;
	start_genworld_worker((struct gen_terrain_ctx *)&ctx->gt, ctx->opts, gts_faults);
}

static void
dec_faults(struct ui_ctx *ctx)
{
	ctx->opts->faults /= 2;
	start_genworld_worker((struct gen_terrain_ctx *)&ctx->gt, ctx->opts, gts_faults);
}

typedef void ((*menu_cb_func)(struct ui_ctx *ctx));

struct menu_cb_ctx {
	struct ui_ctx *ctx;
	menu_cb_func func;
};

static void
render_menu_cb(void *_ctx, bool hvr, float x, float y, const char *str)
{
	struct menu_cb_ctx *ctx = _ctx;
	vec4 clr = { 0, 0, 0, 1 };
	if (hvr) {
		if (ctx->ctx->mb & 1) {
			ctx->ctx->mb &= ~1;
			ctx->func(ctx->ctx);
			clr[2] = 1.0f;
		} else {
			clr[1] = 1.0f;
		}
	} else {
		clr[0] = clr[1] = clr[2] = 1.0f;
	};

	gl_write_string(x, y, 1.0, clr, str);
}

static void
render_menu(struct ui_ctx *ctx)
{
	float sx, sy;
	struct menu_cb_ctx mctx = { ctx };
	struct pointf mouse = { ctx->mousex / SCALE, (ctx->win.height - ctx->mousey) / SCALE };

	mctx.func = shuffle_seed;
	screen_coords_to_text_coords(0, -1, &sx, &sy);
	gl_printf(sx, sy, ta_left, "seed: ");
	gl_mprintf(6, -1, ta_left, &mouse, &mctx, render_menu_cb, "%10d", ctx->gt.terra.opts.seed);

	screen_coords_to_text_coords(0, -2, &sx, &sy);
	gl_printf(sx, sy, ta_left, "pts:  %8d", ctx->gt.terra.opts.points);

	mctx.func = dec_points;
	gl_mprintf(14, -2, ta_left, &mouse, &mctx, render_menu_cb, "/");

	mctx.func = inc_points;
	gl_mprintf(15, -2, ta_left, &mouse, &mctx, render_menu_cb, "*");

	screen_coords_to_text_coords(0, -3, &sx, &sy);
	gl_printf(sx, sy, ta_left, "faults:  %5d", ctx->gt.terra.opts.faults);

	mctx.func = dec_faults;
	gl_mprintf(14, -3, ta_left, &mouse, &mctx, render_menu_cb, "/");

	mctx.func = inc_faults;
	gl_mprintf(15, -3, ta_left, &mouse, &mctx, render_menu_cb, "*");

	/* screen_coords_to_text_coords( */
	/* 	ctx->mousex / SCALE - 0.5, */
	/* 	-ctx->mousey / SCALE - 0.5, */
	/* 	&sx, &sy); */
	/* gl_printf(sx, sy, ta_left, "*"); */
}

void
genworld_interactive(struct worldgen_opts *opts)
{
	struct ui_ctx ctx = { 0 };
	long slept_ns = 0;
	struct timespec tick_st;
	clock_gettime(CLOCK_MONOTONIC, &tick_st);

	if (!genworld_interactive_setup(&ctx)) {
		return;
	} else if (!render_terrain_init(&ctx)) {
		return;
	}

	ctx.opts = opts;
	init_genworld_worker((struct gen_terrain_ctx *)&ctx.gt);
	start_genworld_worker((struct gen_terrain_ctx *)&ctx.gt, opts, gts_triangulate);

	while (1) {
		glViewport(0, 0, ctx.win.width, ctx.win.height);
		glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

		glfwPollEvents();

		render_terrain_setup(&ctx);
		render_terrain(&ctx);

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
