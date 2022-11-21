#include "posix.h"

#include <assert.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "client/client.h"
#include "client/input_handler.h"
#include "client/ui/common.h"
#include "client/ui/gl/globals.h"
#include "client/ui/gl/input.h"
#include "client/ui/gl/render.h"
#include "client/ui/gl/render/chunks.h"
#include "client/ui/gl/ui.h"
#include "shared/sim/tiles.h"
#include "shared/ui/gl/menu.h"
#include "shared/util/log.h"
#include "shared/util/util.h"

#define LOOK_SENS 0.0018
#define SCROLL_SENS 3.0f

static bool wireframe = false;

enum gl_direction { direction_forward, direction_back, direction_left, direction_right, direction_up, direction_down };

static void
handle_flying(enum gl_direction dir, float speed)
{
	vec4 v1;
	memcpy(v1, cam.tgt, sizeof(float) * 4);

	switch (dir) {
	case direction_forward:
		vec_scale(v1, speed);
		vec_sub(cam.pos, v1);
		break;
	case direction_back:
		vec_scale(v1, speed);
		vec_add(cam.pos, v1);
		break;
	case direction_left:
		vec_cross(v1, cam.up);
		vec_normalize(v1);
		vec_scale(v1, speed);
		vec_add(cam.pos, v1);
		break;
	case direction_right:
		vec_cross(v1, cam.up);
		vec_normalize(v1);
		vec_scale(v1, speed);
		vec_sub(cam.pos, v1);
		break;
	default: break;
	}
}

static void
cmd_fly(struct client *cli, uint32_t c)
{
	handle_flying(c, 0.1f);
}

static void
cmd_view_rotate(struct client *cli, uint32_t c)
{
	switch (c) {
	case direction_right:
		cli->ref.angle += 0.015f;
		break;
	case direction_left:
		cli->ref.angle -= 0.015f;
		break;
	case direction_up:
		cam.pitch -= 0.01f;
		break;
	case direction_down:
		cam.pitch += 0.01f;
		break;
	default: break;
	}
}

static void
handle_held_keys(struct gl_ui_ctx *ctx, struct client *cli)
{
	uint32_t i;
	uint16_t key;

	if (cli->im == im_cmd) {
		return;
	}

	for (i = 0; i < ctx->win->keyboard.held_len; ++i) {
		key = ctx->win->keyboard.held[i];
		if ((key == skc_mb1 || key == skc_mb2 || key == skc_mb3)
		    && !ctx->cursor_on_world) {
			continue;
		}
		input_handle_key(cli, key, ctx->win->keyboard.mod, key_action_held);
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
		cam.unlocked = !cam.unlocked;
		if (!cam.unlocked) {
			cam.pitch = CAM_PITCH_MAX;
			cam.yaw = CAM_YAW;
		}
		break;
	case ui_const_look_angle:
	{
		break;
	}
	case ui_const_debug_hud:
		ctx->debug_hud = !ctx->debug_hud;
		break;
	}
}

static const struct gl_ui_ctx *ui_ctx; // global used only for the below callbacks
static void
handle_typed_key(void *_ctx, uint8_t mod, uint8_t k, uint8_t action)
{
	struct client *cli = _ctx;

	if (!ui_ctx->cursor_on_world) {
		switch (k) {
		case skc_mb1: case skc_mb2: case skc_mb3: return;
		}
	}

	input_handle_key(cli, k, mod, action);
}

static void
handle_typed_text(void *_ctx, uint32_t codepoint)
{
}

static void
handle_gl_mouse(struct gl_ui_ctx *ctx, struct client *cli)
{
	if (cam.unlocked) {
		cam.yaw += ctx->win->mouse.dx * LOOK_SENS;
		cam.pitch += ctx->win->mouse.dy * LOOK_SENS;
	} else {
		float sens;

		sens = cli->opts->ui_cfg.mouse_sensitivity * 0.00005;
		ctx->sc_cursor.x = fclamp(ctx->sc_cursor.x + ctx->win->mouse.dx * sens, 0.0f, 1.0f);
		ctx->sc_cursor.y = fclamp(ctx->sc_cursor.y + ctx->win->mouse.dy * sens, 0.0f, 1.0f);

		cam.pos[1] += -2 * floorf(ctx->win->mouse.scroll * SCROLL_SENS);
	}

	ctx->win->mouse.scroll = 0;
	ctx->win->mouse.still = true;
}

void
register_input_cfg_data(void)
{
	static const struct input_command_name command_names[] = {
		{ "gl_ui_toggle", cmd_gl_ui_toggle },
		{ "fly",  cmd_fly, },
		{ "view_rotate",  cmd_view_rotate, },
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
		"forward",                   direction_forward,
		"backward",                  direction_back,
		"left",                      direction_left,
		"right",                     direction_right,
		"up",                        direction_up,
		"down",                      direction_down,
	};

	register_input_commands(command_names);
	register_input_constants(&gl_ui_constants);
}

void
set_input_callbacks(struct gl_ui_ctx *ctx)
{
	ui_ctx = ctx;
	ctx->win->key_input_callback = handle_typed_key;
	ctx->win->text_input_callback = handle_typed_text;
	cam.changed = true; // why?
}

void
gl_ui_handle_input(struct gl_ui_ctx *ctx, struct client *cli)
{
	struct camera ocam = cam;

	gl_win_poll_events(cli);

	handle_held_keys(ctx, cli);

	handle_gl_mouse(ctx, cli);

	if (!cam.unlocked) {
		const float terrain_h = get_height_at(&cli->world->chunks, &(struct point){ cam.pos[0], cam.pos[2] }) + 10,
			    min_h = maxf(CAM_HEIGHT_MIN, terrain_h);

		if (cam.pos[1] > CAM_HEIGHT_MAX) {
			cam.pos[1] = CAM_HEIGHT_MAX;
		} else if (cam.pos[1] < min_h) {
			cam.pos[1] = min_h;
		}

		if (cam.pitch > CAM_PITCH_MAX) {
			cam.pitch = CAM_PITCH_MAX;
		} else if (cam.pitch < CAM_PITCH_MIN) {
			cam.pitch = CAM_PITCH_MIN;
		}
	} else {
		if (cam.pitch > DEG_90) {
			cam.pitch = DEG_90;
		} else if (cam.pitch < -DEG_90) {
			cam.pitch = -DEG_90;
		}

		// fps?
		/* struct point p = { cam.pos[0], cam.pos[2] }; */
		/* float dz = (get_height_at(&cli->world->chunks, &p) + 2.0f) - cam.pos[1]; */
		/* cam.pos[1] += dz * 0.05f; */
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
