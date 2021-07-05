#include "posix.h"

#include <assert.h>
#include <math.h>
#include <string.h>

#include "client/client.h"
#include "client/input/action_handler.h"
#include "client/input/handler.h"
#include "client/input/helpers.h"
#include "client/ui/opengl/globals.h"
#include "client/ui/opengl/input.h"
#include "client/ui/opengl/input_types.h"
#include "client/ui/opengl/ui.h"
#include "shared/util/log.h"
#include "shared/util/util.h"

#define LOOK_SENS 0.0018
#define SCROLL_SENS 3.0f

static bool wireframe = false;

enum okc_input_type {
	okcit_oneshot, /* default */
	okcit_hold,
};

static const enum okc_input_type okc_input_type[opengl_key_command_count] = {
	[okc_fly_forward] = okcit_hold,
	[okc_fly_left] = okcit_hold,
	[okc_fly_right] = okcit_hold,
	[okc_fly_back] = okcit_hold,
};

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

static void
handle_okc(struct opengl_ui_ctx *ctx, enum opengl_key_command action)
{
	switch (action) {
	case okc_toggle_wireframe:
		if ((wireframe = !wireframe)) {
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		} else {
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		}
		break;
	case okc_toggle_camera_lock:
		if (!(cam.unlocked = !cam.unlocked)) {
			cam.pitch = CAM_PITCH;
			cam.yaw = DEG_90;

			ctx->im_keyboard_new = oim_normal;
			ctx->im_mouse_new    = oim_normal;
		} else {
			ctx->im_keyboard_new = oim_flying;
			ctx->im_mouse_new    = oim_flying;
		}
		break;
	case okc_toggle_look_angle:
	{
		float a = fabs(ctx->opts.cam_pitch_max - cam.pitch),
		      b = fabs(ctx->opts.cam_pitch_min - cam.pitch);

		ctx->cam_animation.pitch = a > b ?  1 : -1;
		break;
	}
	case okc_fly_forward:
		handle_flying(fly_forward, 2.0f);
		break;
	case okc_fly_left:
		handle_flying(fly_left, 2.0f);
		break;
	case okc_fly_right:
		handle_flying(fly_right, 2.0f);
		break;
	case okc_fly_back:
		handle_flying(fly_back, 2.0f);
		break;
	case opengl_key_command_count:
		break;
	case okc_toggle_render_step_ents:
	case okc_toggle_render_step_selection:
	case okc_toggle_render_step_chunks:
	case okc_toggle_render_step_shadows:
	case okc_toggle_render_step_reflections:
		ctx->rendering_disabled ^= 1 << (action - okc_toggle_render_step_ents);
		break;
	case okc_toggle_debug_hud:
		ctx->debug_hud = !ctx->debug_hud;
		break;
	}
}

void
handle_held_keys(struct opengl_ui_ctx *ctx)
{
	uint32_t i, j;
	uint16_t key;

	for (i = 0; i < ctx->win->keyboard.held_len; ++i) {
		key = ctx->win->keyboard.held[i];
		for (j = 0; j < ctx->input_maps[ctx->im_keyboard].keyboard_len; ++j) {
			struct opengl_key_map *km = &ctx->input_maps[ctx->im_keyboard].keyboard[j];

			if (!(km->key == key && km->mod == ctx->win->keyboard.mod
			      && okc_input_type[km->action] == okcit_hold)) {
				continue;
			}

			handle_okc(ctx, km->action);
		}
	}
}

static void
handle_typed_key(void *_ctx, uint8_t mod, uint8_t k)
{
	struct opengl_ui_ctx *ctx = _ctx;
	ctx->last_key = k;

	uint32_t i;

	LOG_I(log_misc, "key: %x, %d", mod, k);

	for (i = 0; i < ctx->input_maps[ctx->im_keyboard].keyboard_len; ++i) {
		struct opengl_key_map *km = &ctx->input_maps[ctx->im_keyboard].keyboard[i];


		if (!(km->key == k && km->mod == ctx->win->keyboard.mod
		      && okc_input_type[km->action] == okcit_oneshot)) {
			continue;
		}

		handle_okc(ctx, km->action);

		break;
	}

	if ((*ctx->km = handle_input(*ctx->km, k, ctx->cli)) == NULL) {
		*ctx->km = &ctx->cli->keymaps[ctx->cli->im];
	}
}

static void
move_cursor(struct client *cli, struct opengl_ui_ctx *ctx)
{
	trigger_cmd_with_num(kc_cursor_right, cli, floor(ctx->win->mouse.cursx));
	trigger_cmd_with_num(kc_cursor_down, cli, floor(ctx->win->mouse.cursy));
	trigger_cmd(kc_center_cursor, cli);
}

#define MOUSE_MOVED_THRESH 0.01f

void
handle_gl_mouse(struct opengl_ui_ctx *ctx, struct client *cli)
{
	static uint8_t buttons_dragging = 0;
	uint8_t released;
	uint32_t i;
	bool mouse_moved;

	if (ctx->win->mouse.still) {
		ctx->win->mouse.dx = 0;
		ctx->win->mouse.dy = 0;

		goto skip_mouse;
	} else {
		ctx->win->mouse.dx = ctx->win->mouse.x - ctx->win->mouse.lx;
		ctx->win->mouse.dy = ctx->win->mouse.y - ctx->win->mouse.ly;

		ctx->win->mouse.lx = ctx->win->mouse.x;
		ctx->win->mouse.ly = ctx->win->mouse.y;

		if (!ctx->win->mouse.init) {
			ctx->win->mouse.init = true;

			goto skip_mouse;
		}
	}

	ctx->win->mouse.scaled_dx = ctx->win->mouse.dx * cam.pos[1] * 0.001;
	ctx->win->mouse.scaled_dy = ctx->win->mouse.dy * cam.pos[1] * 0.001;

	ctx->win->mouse.cursx += ctx->win->mouse.scaled_dx;
	ctx->win->mouse.cursy += ctx->win->mouse.scaled_dy;

	mouse_moved = fabs(ctx->win->mouse.dx) > MOUSE_MOVED_THRESH
		      || fabs(ctx->win->mouse.dy) > MOUSE_MOVED_THRESH;

	released = ctx->win->mouse.old_buttons & ~(ctx->win->mouse.buttons | buttons_dragging);
	/* released = ctx->mouse.old_buttons & ~ctx->mouse.buttons; */

	for (i = 0; i < ctx->input_maps[ctx->im_mouse].mouse_len; ++i) {
		struct opengl_mouse_map *mm = &ctx->input_maps[ctx->im_mouse].mouse[i];

		switch (mm->type) {
		case mmt_click:
			if (!(mm->mod == ctx->win->keyboard.mod
			      && (mm->button == released))) {
				goto inactive;
			}

			if (mm->action.click.is_opengl_kc) {
				handle_okc(ctx, mm->action.click.kc);
			} else {
				trigger_cmd(mm->action.click.kc, cli);
			}
			break;
		case mmt_scroll:
			if (!(ctx->win->mouse.scroll
			      && mm->mod == ctx->win->keyboard.mod
			      && (mm->button == ctx->win->mouse.buttons))) {
				goto inactive;
			}

			switch (mm->action.scroll) {
			case mas_noop:
				break;
			case mas_zoom:
				cam.pos[1] += floorf(ctx->win->mouse.scroll * SCROLL_SENS);
				break;
			case mas_quick_zoom:
				cam.pos[1] += 2 * floorf(ctx->win->mouse.scroll * SCROLL_SENS);
				break;

			}
			break;
		case mmt_drag:
			if (!(mouse_moved
			      && mm->mod == ctx->win->keyboard.mod
			      && (mm->button == ctx->win->mouse.buttons))) {
				if (mm->active) {
					buttons_dragging &= ~mm->button;

					switch (mm->action.drag) {
					default:
						break;
					}
				}
				goto inactive;
			}

			switch (mm->action.drag) {
			case mad_noop:
				break;
			case mad_point_camera:
				cam.yaw += ctx->win->mouse.dx * LOOK_SENS;
				cam.pitch += ctx->win->mouse.dy * LOOK_SENS;
				break;
			case mad_move_view:
				move_viewport(cli, floor(ctx->win->mouse.cursx), floor(ctx->win->mouse.cursy));

				constrain_cursor(&ctx->ref, &cli->cursor);
				break;
			case mad_move_cursor_neutral:
				move_cursor(cli, ctx);
				ctx->cli->action = act_neutral;
				break;
			case mad_move_cursor_destroy:
				move_cursor(cli, ctx);
				ctx->cli->action = act_destroy;
				break;
			case mad_move_cursor_create:
				move_cursor(cli, ctx);
				ctx->cli->action = act_create;
				break;
			}

			buttons_dragging |= mm->button;
			mm->active = true;
			break;
		}

		continue;
inactive:
		mm->active = false;
	}

	ctx->win->mouse.scroll = 0;
	ctx->win->mouse.cursx -= floor(ctx->win->mouse.cursx);
	ctx->win->mouse.cursy -= floor(ctx->win->mouse.cursy);

	ctx->win->mouse.still = true;

skip_mouse:
	ctx->win->mouse.old_buttons = ctx->win->mouse.buttons;
}

void
set_input_callbacks(struct opengl_ui_ctx *ctx)
{
	ctx->win->keyboard_oneshot_callback = handle_typed_key;
	cam.changed = true; // why?
}
