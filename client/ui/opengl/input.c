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

enum mouse_buttons mouse_buttons_tl[] = {
	[GLFW_MOUSE_BUTTON_1] = mb_1,
	[GLFW_MOUSE_BUTTON_2] = mb_2,
	[GLFW_MOUSE_BUTTON_3] = mb_3,
	[GLFW_MOUSE_BUTTON_4] = mb_4,
	[GLFW_MOUSE_BUTTON_5] = mb_5,
	[GLFW_MOUSE_BUTTON_6] = mb_6,
	[GLFW_MOUSE_BUTTON_7] = mb_7,
	[GLFW_MOUSE_BUTTON_8] = mb_8,
};

static bool wireframe = false;

static int
transform_glfw_key(struct opengl_ui_ctx *ctx, int k)
{
	switch (k) {
	case GLFW_KEY_UP:
		return skc_up;
	case GLFW_KEY_DOWN:
		return skc_down;
	case GLFW_KEY_LEFT:
		return skc_left;
	case GLFW_KEY_RIGHT:
		return skc_right;
	case GLFW_KEY_ENTER:
		return '\n';
	case GLFW_KEY_TAB:
		return '\t';
	case GLFW_KEY_F1:
		return skc_f1;
	case GLFW_KEY_F2:
		return skc_f2;
	case GLFW_KEY_F3:
		return skc_f3;
	case GLFW_KEY_F4:
		return skc_f4;
	case GLFW_KEY_F5:
		return skc_f5;
	case GLFW_KEY_F6:
		return skc_f6;
	case GLFW_KEY_F7:
		return skc_f7;
	case GLFW_KEY_F8:
		return skc_f8;
	case GLFW_KEY_F9:
		return skc_f9;
	case GLFW_KEY_F10:
		return skc_f10;
	case GLFW_KEY_F11:
		return skc_f11;
	case GLFW_KEY_F12:
		return skc_f12;
	case GLFW_KEY_BACKSPACE:
		return '\b';
	case GLFW_KEY_PAGE_UP:
		return skc_pgup;
	case GLFW_KEY_PAGE_DOWN:
		return skc_pgdn;
	case GLFW_KEY_HOME:
		return skc_home;
	case GLFW_KEY_END:
		return skc_end;
	default:
		return k;
	}
}

static void
handle_typed_key(struct opengl_ui_ctx *ctx, uint8_t k)
{
	ctx->last_key = k;

	if ((*ctx->km = handle_input(*ctx->km, k, ctx->cli)) == NULL) {
		*ctx->km = &ctx->cli->keymaps[ctx->cli->im];
	}
}

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
	case okc_release_mouse:
		glfwSetInputMode(ctx->win.win, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		ctx->im_mouse_new = oim_released;
		break;
	case okc_capture_mouse:
		glfwSetInputMode(ctx->win.win, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
		ctx->im_mouse_new = oim_normal;
		break;
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

static void
key_callback(GLFWwindow *window, int32_t key, int32_t _scancode, int32_t action, int32_t _mods)
{
	if (key < 0) {
		L("skipping unknown key: %d", _scancode);
		return;
	}

	struct opengl_ui_ctx *ctx = glfwGetWindowUserPointer(window);
	int32_t transformed;
	uint32_t j;
	enum modifier_types mod;

	switch (key) {
	case GLFW_KEY_RIGHT_SHIFT:
	case GLFW_KEY_LEFT_SHIFT:
		mod = mod_shift;
		break;
	case GLFW_KEY_RIGHT_CONTROL:
	case GLFW_KEY_LEFT_CONTROL:
		mod = mod_ctrl;
		break;
	default:
		goto no_mod;
	}

	if (action == GLFW_PRESS || action == GLFW_REPEAT) {
		ctx->keyboard.mod |= mod;
	} else {
		ctx->keyboard.mod &= ~mod;
	}

	return;
no_mod:
	switch (action) {
	case GLFW_PRESS:
		if (ctx->keyboard.held_len < INPUT_KEY_BUF_MAX ) {
			ctx->keyboard.held[ctx->keyboard.held_len] = key;

			++ctx->keyboard.held_len;
		}
		break;
	case GLFW_REPEAT:
		/* nothing */
		break;
	case GLFW_RELEASE:
		assert(ctx->keyboard.held_len);

		for (j = 0; j < ctx->keyboard.held_len; ++j) {
			if (ctx->keyboard.held[j] == key) {
				if (j < (uint32_t)(ctx->keyboard.held_len - 1)) {
					ctx->keyboard.held[j] = ctx->keyboard.held[ctx->keyboard.held_len - 1];
				}

				break;
			}
		}

		assert(j != ctx->keyboard.held_len);

		--ctx->keyboard.held_len;

		return;
	}

	for (j = 0; j < ctx->input_maps[ctx->im_keyboard].keyboard_len; ++j) {
		struct opengl_key_map *km = &ctx->input_maps[ctx->im_keyboard].keyboard[j];

		if (!(km->key == key && km->mod == ctx->keyboard.mod
		      && okc_input_type[km->action] == okcit_oneshot)) {
			continue;
		}

		handle_okc(ctx, km->action);

		break;
	}

	if ((transformed = transform_glfw_key(ctx, key)) != key) {
		handle_typed_key(ctx, transformed);
	}
}

static void
char_callback(GLFWwindow* window, uint32_t codepoint)
{
	struct opengl_ui_ctx *ctx = glfwGetWindowUserPointer(window);

	if (codepoint > 256) {
		/* we don't support non-ascii codepoints :( */
		return;
	}

	handle_typed_key(ctx, codepoint);
}

void
handle_held_keys(struct opengl_ui_ctx *ctx)
{
	uint32_t i, j;
	uint16_t key;

	for (i = 0; i < ctx->keyboard.held_len; ++i) {
		key = ctx->keyboard.held[i];
		for (j = 0; j < ctx->input_maps[ctx->im_keyboard].keyboard_len; ++j) {
			struct opengl_key_map *km = &ctx->input_maps[ctx->im_keyboard].keyboard[j];

			if (!(km->key == key && km->mod == ctx->keyboard.mod
			      && okc_input_type[km->action] == okcit_hold)) {
				continue;
			}

			handle_okc(ctx, km->action);
		}
	}
}


static void
mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	struct opengl_ui_ctx *ctx = glfwGetWindowUserPointer(window);

	ctx->mouse.x = xpos;
	ctx->mouse.y = ypos;

	ctx->mouse.still = false;
}

static void
scroll_callback(GLFWwindow* window, double xoff, double yoff)
{
	struct opengl_ui_ctx *ctx = glfwGetWindowUserPointer(window);

	ctx->mouse.scroll = yoff;

	ctx->mouse.still = false;
}

static void
mouse_button_callback(GLFWwindow* window, int button, int action, int _mods)
{
	struct opengl_ui_ctx *ctx = glfwGetWindowUserPointer(window);

	assert(button < 8);

	if (action == GLFW_PRESS) {
		ctx->mouse.buttons |= mouse_buttons_tl[button];
	} else {
		ctx->mouse.buttons &= ~mouse_buttons_tl[button];
	}

	ctx->mouse.still = false;
}

static void
move_cursor(struct client *cli, struct opengl_ui_ctx *ctx)
{
	trigger_cmd_with_num(kc_cursor_right, cli, floor(ctx->mouse.cursx));
	trigger_cmd_with_num(kc_cursor_down, cli, floor(ctx->mouse.cursy));
}

#define MOUSE_MOVED_THRESH 0.01f

void
handle_gl_mouse(struct opengl_ui_ctx *ctx, struct client *cli)
{
	static uint8_t buttons_dragging = 0;
	uint8_t released;
	uint32_t i;
	bool mouse_moved;

	if (ctx->mouse.still) {
		ctx->mouse.dx = 0;
		ctx->mouse.dy = 0;

		goto skip_mouse;
	} else {
		ctx->mouse.dx = ctx->mouse.x - ctx->mouse.lx;
		ctx->mouse.dy = ctx->mouse.y - ctx->mouse.ly;

		ctx->mouse.lx = ctx->mouse.x;
		ctx->mouse.ly = ctx->mouse.y;

		if (!ctx->mouse.init) {
			ctx->mouse.init = true;

			goto skip_mouse;
		}
	}

	ctx->mouse.scaled_dx = ctx->mouse.dx * cam.pos[1] * 0.001;
	ctx->mouse.scaled_dy = ctx->mouse.dy * cam.pos[1] * 0.001;

	ctx->mouse.cursx += ctx->mouse.scaled_dx;
	ctx->mouse.cursy += ctx->mouse.scaled_dy;

	mouse_moved = fabs(ctx->mouse.dx) > MOUSE_MOVED_THRESH
		      || fabs(ctx->mouse.dy) > MOUSE_MOVED_THRESH;

	released = ctx->mouse.old_buttons & ~(ctx->mouse.buttons | buttons_dragging);
	/* released = ctx->mouse.old_buttons & ~ctx->mouse.buttons; */

	for (i = 0; i < ctx->input_maps[ctx->im_mouse].mouse_len; ++i) {
		struct opengl_mouse_map *mm = &ctx->input_maps[ctx->im_mouse].mouse[i];

		switch (mm->type) {
		case mmt_click:
			if (!(mm->mod == ctx->keyboard.mod
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
			if (!(ctx->mouse.scroll
			      && mm->mod == ctx->keyboard.mod
			      && (mm->button == ctx->mouse.buttons))) {
				goto inactive;
			}

			switch (mm->action.scroll) {
			case mas_noop:
				break;
			case mas_zoom:
				cam.pos[1] += floorf(ctx->mouse.scroll * SCROLL_SENS);
				break;
			case mas_quick_zoom:
				cam.pos[1] += 2 * floorf(ctx->mouse.scroll * SCROLL_SENS);
				break;

			}
			break;
		case mmt_drag:
			if (!(mouse_moved
			      && mm->mod == ctx->keyboard.mod
			      && (mm->button == ctx->mouse.buttons))) {
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
				cam.yaw += ctx->mouse.dx * LOOK_SENS;
				cam.pitch += ctx->mouse.dy * LOOK_SENS;
				break;
			case mad_move_view:
				move_viewport(cli, floor(ctx->mouse.cursx), floor(ctx->mouse.cursy));

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

	ctx->mouse.scroll = 0;
	ctx->mouse.cursx -= floor(ctx->mouse.cursx);
	ctx->mouse.cursy -= floor(ctx->mouse.cursy);

	ctx->mouse.still = true;

skip_mouse:
	ctx->mouse.old_buttons = ctx->mouse.buttons;
}

void
set_input_callbacks(struct GLFWwindow *window)
{
	glfwSetKeyCallback(window, key_callback);
	glfwSetCharCallback(window, char_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);
	glfwSetMouseButtonCallback(window, mouse_button_callback);

	cam.changed = true;
}
