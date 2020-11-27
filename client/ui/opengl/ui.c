#include "posix.h"

#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "client/ui/opengl/globals.h"
#include "client/ui/opengl/input.h"
#include "client/ui/opengl/loaders/color_cfg.h"
#include "client/ui/opengl/render.h"
#include "client/ui/opengl/ui.h"
#include "shared/opengl/window.h"
#include "shared/util/log.h"

static void
resize_callback(struct GLFWwindow *win, int width, int height)
{
	struct opengl_ui_ctx *ctx = glfwGetWindowUserPointer(win);

	ctx->win.width = width;
	ctx->win.height = height;

	ctx->win.resized = true;
}

bool
opengl_ui_init(struct opengl_ui_ctx *ctx)
{
	int x, y;

	ctx->time.sun_theta_tgt = 6.872234; /* 10:45 */

	if (!(ctx->window = init_window())) {
		goto free_exit;
	}

	glfwSetWindowUserPointer(ctx->window, ctx);

	/* load opengl cfg */
	if (!parse_opengl_cfg(&ctx->opts)) {
		goto free_exit;
	}

	/* load color config */
	if (!color_cfg()) {
		goto free_exit;
	}

	/* Set callbacks */
	set_input_callbacks(ctx->window);
	glfwSetFramebufferSizeCallback(ctx->window, resize_callback);

	/* set input mode */
	glfwSetInputMode(ctx->window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	/* setup rendering */
	if (!opengl_ui_render_setup(ctx)) {
		goto free_exit;
	}

	glClearColor(colors.tile_fg[tile_deep_water][0],
		colors.tile_fg[tile_deep_water][1],
		colors.tile_fg[tile_deep_water][2], 1.0);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glEnable(GL_FRAMEBUFFER_SRGB);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);

	glfwGetWindowSize(ctx->window, &x, &y);
#ifdef __APPLE__
	/* HACK macOS has a black screen before being resized */

	glfwSwapBuffers(ctx->window);

	x += 1; y += 1;
	glfwSetWindowSize(ctx->window, x, y);
#endif
	ctx->win.width = x;
	ctx->win.height = y;

	darr_init(&ctx->debug_hl_points, sizeof(struct point));

	return true;
free_exit:
	opengl_ui_deinit(ctx);
	return false;
}

void
opengl_ui_handle_input(struct opengl_ui_ctx *ctx, struct keymap **km,
	struct hiface *hf)
{
	struct camera ocam = cam;

	ctx->oim = hf->im;
	ctx->okm = ctx->ckm;
	ctx->km = km;
	/* TODO: only need to do this once */
	ctx->hf = hf;

	glfwPollEvents();
	handle_held_keys(ctx);
	handle_gl_mouse(ctx, hf);

	ctx->im_mouse = ctx->im_mouse_new;
	ctx->im_keyboard = ctx->im_keyboard_new;

	ctx->ckm = *km;

	if (ctx->cam_animation.pitch) {
		cam.pitch += ctx->cam_animation.pitch
			     * (ctx->opts.cam_pitch_max - ctx->opts.cam_pitch_min) * 0.05;
	}

	if (!cam.unlocked) {
		if (cam.pos[1] > ctx->opts.cam_height_max) {
			cam.pos[1] = ctx->opts.cam_height_max;
		} else if (cam.pos[1] < ctx->opts.cam_height_min) {
			cam.pos[1] = ctx->opts.cam_height_min;
		}

		if (cam.pitch > ctx->opts.cam_pitch_max) {
			cam.pitch = ctx->opts.cam_pitch_max;

			ctx->cam_animation.pitch = 0;
		} else if (cam.pitch < ctx->opts.cam_pitch_min) {
			cam.pitch = ctx->opts.cam_pitch_min;

			ctx->cam_animation.pitch = 0;
		}
	} else {
		if (cam.pitch > DEG_90) {
			cam.pitch = DEG_90;
		} else if (cam.pitch < -DEG_90) {
			cam.pitch = -DEG_90;
		}
	}

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
	glfwTerminate();
}
