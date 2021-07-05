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

void
opengl_ui_handle_input(struct opengl_ui_ctx *ctx, struct client *cli)
{
	struct camera ocam = cam;

	ctx->oim = cli->im;
	ctx->okm = ctx->ckm;
	ctx->km = &cli->ckm;
	/* TODO: only need to do this once */
	ctx->cli = cli;

	win_poll_events();
	handle_held_keys(ctx);
	handle_gl_mouse(ctx, cli);

	if (cli->state & csf_paused) {
		ctx->im_mouse_new = oim_released;
	} else if (ctx->im_mouse == oim_released) {
		ctx->im_mouse_new = oim_normal;
	}

	if (ctx->im_mouse != ctx->im_mouse_new) {
		switch (ctx->im_mouse_new) {
		case oim_released:
			win_set_cursor_display(true);
			break;
		case oim_flying:
		case oim_normal:
			win_set_cursor_display(false);
			break;
		default:
			break;
		}

		ctx->im_mouse = ctx->im_mouse_new;
	}
	ctx->im_keyboard = ctx->im_keyboard_new;

	ctx->ckm = cli->ckm;

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

	/* if (glfwWindowShouldClose(ctx->win->win)) { */
	/* 	cli->run = false; */
	/* } else if (!cli->run) { */
	/* 	glfwSetWindowShouldClose(ctx->win->win, 1); */
	/* } */
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
