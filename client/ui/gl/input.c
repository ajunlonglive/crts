#include "posix.h"

#include <assert.h>
#include <math.h>
#include <string.h>

#include "client/client.h"
#include "client/input_handler.h"
#include "client/ui/common.h"
#include "client/ui/gl/globals.h"
#include "client/ui/gl/input.h"
#include "client/ui/gl/ui.h"
#include "shared/ui/gl/menu.h"
#include "shared/util/log.h"
#include "shared/util/util.h"

#define LOOK_SENS 0.0018
#define SCROLL_SENS 3.0f

static bool wireframe = false;

enum flydir { fly_forward, fly_back, fly_left, fly_right, };

static void
handle_flying(enum flydir dir, float speed)
{
	vec4 v1;
	memcpy(v1, cam.tgt, sizeof(float) * 4);

	switch (dir) {
	case fly_forward:
		vec4_scale(v1, speed);
		vec4_sub(cam.pos, v1);
		break;
	case fly_back:
		vec4_scale(v1, speed);
		vec4_add(cam.pos, v1);
		break;
	case fly_left:
		vec4_cross(v1, cam.up);
		vec4_normalize(v1);
		vec4_scale(v1, speed);
		vec4_add(cam.pos, v1);
		break;
	case fly_right:
		vec4_cross(v1, cam.up);
		vec4_normalize(v1);
		vec4_scale(v1, speed);
		vec4_sub(cam.pos, v1);
		break;
	}
}

void
handle_held_keys(struct gl_ui_ctx *ctx)
{
	uint32_t i;
	uint16_t key;

	for (i = 0; i < ctx->win->keyboard.held_len; ++i) {
		key = ctx->win->keyboard.held[i];

		L(log_misc, "key: %d", key);

		switch (key) {
		case 'W':
			handle_flying(fly_forward, 2.0f);
			break;
		case 'A':
			handle_flying(fly_left, 2.0f);
			break;
		case 'D':
			handle_flying(fly_right, 2.0f);
			break;
		case 'S':
			handle_flying(fly_back, 2.0f);
			break;
		case 033:
			cam.pitch = CAM_PITCH_MAX;
			cam.yaw = CAM_YAW;
			cam.unlocked = false;
			break;
		}
	}
}

enum gl_ui_constant {
	ui_const_render_step_ents        = 0,
	ui_const_render_step_selection   = 1,
	ui_const_render_step_chunks      = 2,
	ui_const_render_step_shadows     = 3,
	ui_const_render_step_reflections = 4,
	ui_const_wireframe,
	ui_const_camera_lock,
	ui_const_debug_hud,
	ui_const_look_angle,
};

static void
cmd_gl_ui_toggle(struct client *cli, uint32_t c)
{
	struct gl_ui_ctx *ctx = &cli->ui_ctx->gl;

	switch ((enum gl_ui_constant)c) {
	case ui_const_render_step_ents:
	case ui_const_render_step_selection:
	case ui_const_render_step_chunks:
	case ui_const_render_step_shadows:
	case ui_const_render_step_reflections:
		ctx->rendering_disabled ^= 1 << c;
		break;
	case ui_const_wireframe:
		if ((wireframe = !wireframe)) {
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		} else {
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		}
		break;
	case ui_const_camera_lock:
		cam.unlocked = true;
		break;
	case ui_const_look_angle:
	{
		float a = fabs(CAM_PITCH_MAX - cam.pitch),
		      b = fabs(CAM_PITCH_MIN - cam.pitch);

		ctx->cam_animation.pitch = a > b ?  1 : -1;
		break;
	}
	case ui_const_debug_hud:
		ctx->debug_hud = !ctx->debug_hud;
		break;
	}
}

static void
handle_typed_key(void *_ctx, uint8_t mod, uint8_t k, uint8_t action)
{
	struct client *cli = _ctx;

	if (!cam.unlocked) {
		input_handle_key(cli, k, mod, action);
	}
}

void
handle_gl_mouse(struct gl_ui_ctx *ctx, struct client *cli)
{
	const float sens = cli->opts->ui_cfg.mouse_sensitivity * 0.00005,
		    scaled_dx = ctx->win->mouse.dx * cam.pos[1] * sens,
		    scaled_dy = ctx->win->mouse.dy * cam.pos[1] * sens;

	/* ctx->win->mouse.cursx += ctx->win->mouse.scaled_dx; */
	/* ctx->win->mouse.cursy += ctx->win->mouse.scaled_dy; */

	if (cam.unlocked) {
		cam.yaw += ctx->win->mouse.dx * LOOK_SENS;
		cam.pitch += ctx->win->mouse.dy * LOOK_SENS;
	} else {
		input_handle_mouse(cli, scaled_dx, scaled_dy);
	}

	/* case mas_zoom: */
	cam.pos[1] += -2 * floorf(ctx->win->mouse.scroll * SCROLL_SENS);
	/* break; */
	/* case mas_quick_zoom: */
	/* cam.pos[1] += 2 * floorf(ctx->win->mouse.scroll * SCROLL_SENS); */
	/* break; */

	ctx->win->mouse.scroll = 0;
	/* ctx->win->mouse.cursx -= floor(ctx->win->mouse.cursx); */
	/* ctx->win->mouse.cursy -= floor(ctx->win->mouse.cursy); */

	ctx->win->mouse.still = true;
}

void
register_input_cfg_data(void)
{
	static const struct input_command_name command_names[] = {
		{ "gl_ui_toggle", cmd_gl_ui_toggle },
		/* { "fly_forward",  cmd_fly_forward, }, */
		/* { "fly_left",     cmd_fly_left, }, */
		/* { "fly_right",    cmd_fly_right, }, */
		/* { "fly_back",     cmd_fly_back, }, */
		{ 0 },
	};

	static const struct cfg_lookup_table gl_ui_constants = {
		"render_step_ents",          ui_const_render_step_ents,
		"render_step_selection",     ui_const_render_step_selection,
		"render_step_chunks",        ui_const_render_step_chunks,
		"render_step_shadows",       ui_const_render_step_shadows,
		"render_step_reflections",   ui_const_render_step_reflections,
		"wireframe",                 ui_const_wireframe,
		"camera_lock",               ui_const_camera_lock,
		"debug_hud",                 ui_const_debug_hud,
		"look_angle",                ui_const_look_angle,
	};

	register_input_commands(command_names);
	register_input_constants(&gl_ui_constants);
}

void
set_input_callbacks(struct gl_ui_ctx *ctx)
{
	ctx->win->key_input_callback = handle_typed_key;
	cam.changed = true; // why?
}

void
gl_ui_handle_input(struct gl_ui_ctx *ctx, struct client *cli)
{
	struct camera ocam = cam;

	gl_win_poll_events(cli);
	if (cam.unlocked) {
		handle_held_keys(ctx);
	}

	bool new_cursor_state = ctx->cursor_enabled;
	if (cli->state & csf_paused) {
		new_cursor_state = true;
	} else if (ctx->cursor_enabled) {
		new_cursor_state = false;
	}

	if (new_cursor_state != ctx->cursor_enabled) {
		gl_win_set_cursor_display(new_cursor_state);
		ctx->cursor_enabled = new_cursor_state;
	}

	if (!ctx->cursor_enabled) {
		handle_gl_mouse(ctx, cli);
	}

	if (ctx->cam_animation.pitch) {
		cam.pitch += ctx->cam_animation.pitch
			     * (CAM_PITCH_MAX - CAM_PITCH_MIN) * 0.05;
	}

	if (!cam.unlocked) {
		if (cam.pos[1] > CAM_HEIGHT_MAX) {
			cam.pos[1] = CAM_HEIGHT_MAX;
		} else if (cam.pos[1] < CAM_HEIGHT_MIN) {
			cam.pos[1] = CAM_HEIGHT_MIN;
		}

		if (cam.pitch > CAM_PITCH_MAX) {
			cam.pitch = CAM_PITCH_MAX;

			ctx->cam_animation.pitch = 0;
		} else if (cam.pitch < CAM_PITCH_MIN) {
			cam.pitch = CAM_PITCH_MIN;

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
