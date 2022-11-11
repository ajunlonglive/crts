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
	handle_flying(c, 1.0f);
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

	for (i = 0; i < ctx->win->keyboard.held_len; ++i) {
		key = ctx->win->keyboard.held[i];
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

static void
handle_typed_key(void *_ctx, uint8_t mod, uint8_t k, uint8_t action)
{
	struct client *cli = _ctx;

	input_handle_key(cli, k, mod, action);
}

static void
handle_gl_mouse(struct gl_ui_ctx *ctx, struct client *cli)
{
	float sens;

	if (cam.unlocked) {
		cam.yaw += ctx->win->mouse.dx * LOOK_SENS;
		cam.pitch += ctx->win->mouse.dy * LOOK_SENS;
	} else {
		sens = cli->opts->ui_cfg.mouse_sensitivity * 0.00005;
		ctx->sc_cursor.x = fclamp(ctx->sc_cursor.x + ctx->win->mouse.dx * sens, 0.0f, 1.0f);
		ctx->sc_cursor.y = fclamp(ctx->sc_cursor.y + ctx->win->mouse.dy * sens, 0.0f, 1.0f);

		sens = cli->opts->ui_cfg.mouse_sensitivity * 0.00005;
		float scaled_dx = ctx->win->mouse.dx * cam.pos[1] * sens,
		      scaled_dy = ctx->win->mouse.dy * cam.pos[1] * sens;
		input_handle_mouse(cli, scaled_dx, scaled_dy);
	}

	cam.pos[1] += -2 * floorf(ctx->win->mouse.scroll * SCROLL_SENS);

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
	ctx->win->key_input_callback = handle_typed_key;
	cam.changed = true; // why?
}

static bool
trace_cursor_intersect_test(struct gl_ui_ctx *ctx, const float *origin, const float *dir, const struct point *p)
{
	chunk_mesh *ck;
	struct point np = nearest_chunk(p);

	if (!(ck = hdarr_get(&ctx->chunk_meshes, &np))) {
		return 0.0f;
	}

	np = point_sub(p, &np);

	uint32_t idx = (np.x + np.y * CHUNK_SIZE) * 2;

	const uint32_t *t;

	t = &chunk_indices[idx * 3];
	if (ray_intersects_tri(origin, dir, (*ck)[t[0]].pos, (*ck)[t[1]].pos, (*ck)[t[2]].pos)) {
		return true;
	}

	t = &chunk_indices[(idx + 1) * 3];
	if (ray_intersects_tri(origin, dir, (*ck)[t[0]].pos, (*ck)[t[1]].pos, (*ck)[t[2]].pos)) {
		return true;
	}

	return false;
}

static bool
trace_cursor_check_around_point(struct gl_ui_ctx *ctx, struct client *cli,
	const float *behind, const float *dir,  const struct point *c)
{
	struct point p;

	for (p.x = c->x - 1; p.x <= c->x + 1; ++p.x) {
		for (p.y = c->y - 1; p.y <= c->y + 1; ++p.y) {
			if (trace_cursor_intersect_test(ctx, behind, dir, &p)) {
				cli->cursorf.x = p.x;
				cli->cursorf.y = p.x;
				cli->cursor = p;
				return true;
			}

		}
	}


	return false;
}

void
trace_cursor_to_world(struct gl_ui_ctx *ctx, struct client *cli)
{
	float wh = tanf(cam.fov / 2.0f),
	      ww = (wh / (float)ctx->win->sc_height) * (float)ctx->win->sc_width;

	vec4 right = { 0, 1, 0 }, up,
	     dir = { cam.tgt[0], cam.tgt[1], cam.tgt[2], cam.tgt[3] };
	vec_normalize(dir);
	vec_cross(right, dir);
	vec_normalize(right);
	memcpy(up, dir, sizeof(vec4));
	vec_cross(up, right);

	float cx = (ctx->sc_cursor.x * 2.0f - 1.0f) * ww,
	      cy = (ctx->sc_cursor.y * 2.0f - 1.0f) * wh;

	vec_scale(up, -cy);
	vec_scale(right, cx);

	float curs[3] = { 0 };
	vec_add(curs, cam.pos);
	vec_add(curs, up);
	vec_add(curs, right);

	float behind[3] = { 0 };
	vec_add(behind, cam.pos);
	vec_scale(dir, 1.0f);
	vec_add(behind, dir);

	memcpy(dir, curs, sizeof(float) * 3);
	vec_sub(dir, behind);
	vec_normalize(dir);

	float x0 = behind[0], y0 = behind[2], x1 = curs[0], y1 = curs[2];
	float dx = fabsf(x1 - x0), sx = x0 < x1 ? 1 : -1;
	float dy = fabsf(y1 - y0), sy = y0 < y1 ? 1 : -1;
	float err = (dx > dy ? dx : -dy) / 2, e2;

	uint32_t limit = 0;
	while (true) {
		if (trace_cursor_check_around_point(ctx, cli, behind, dir,
			&(struct point) { x0 + 0.5f, y0 + 0.5f })) {
			break;
		}

		if (++limit > 1000) {
			break;
		}

		e2 = err;
		if (e2 > -dx) {
			err -= dy; x0 += sx;
		}
		if (e2 < dy) {
			err += dx; y0 += sy;
		}
	}
}


void
gl_ui_handle_input(struct gl_ui_ctx *ctx, struct client *cli)
{
	struct camera ocam = cam;

	gl_win_poll_events(cli);
	handle_held_keys(ctx, cli);

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

	if (!cam.unlocked) {
		if (cam.pos[1] > CAM_HEIGHT_MAX) {
			cam.pos[1] = CAM_HEIGHT_MAX;
		} else if (cam.pos[1] < CAM_HEIGHT_MIN) {
			cam.pos[1] = CAM_HEIGHT_MIN;
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
