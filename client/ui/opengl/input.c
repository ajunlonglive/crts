#include "posix.h"

#include <assert.h>
#include <math.h>
#include <string.h>

#include "client/hiface.h"
#include "client/input/action_handler.h"
#include "client/input/handler.h"
#include "client/ui/opengl/globals.h"
#include "client/ui/opengl/input.h"
#include "client/ui/opengl/ui.h"
#include "shared/util/log.h"

#define LOOK_SENS 0.0018
#define SCROLL_SENS 3.0f

uint8_t mouse_buttons_tl[] = {
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

static uint8_t shifted_keys[255] = {
	[GLFW_KEY_SPACE] = 32,
	[GLFW_KEY_APOSTROPHE] = '"' /* ' */,
	[GLFW_KEY_COMMA] = '<' /* , */,
	[GLFW_KEY_MINUS] = '_' /* - */,
	[GLFW_KEY_PERIOD] = '>' /* . */,
	[GLFW_KEY_SLASH] = '?' /* / */,
	[GLFW_KEY_0] = ')',
	[GLFW_KEY_1] = '!',
	[GLFW_KEY_2] = '@',
	[GLFW_KEY_3] = '#',
	[GLFW_KEY_4] = '$',
	[GLFW_KEY_5] = '%',
	[GLFW_KEY_6] = '^',
	[GLFW_KEY_7] = '&',
	[GLFW_KEY_8] = '*',
	[GLFW_KEY_9] = '(',
	[GLFW_KEY_SEMICOLON] = ':' /* ; */,
	[GLFW_KEY_EQUAL] = '+' /* = */,
	[GLFW_KEY_A] = 65,
	[GLFW_KEY_B] = 66,
	[GLFW_KEY_C] = 67,
	[GLFW_KEY_D] = 68,
	[GLFW_KEY_E] = 69,
	[GLFW_KEY_F] = 70,
	[GLFW_KEY_G] = 71,
	[GLFW_KEY_H] = 72,
	[GLFW_KEY_I] = 73,
	[GLFW_KEY_J] = 74,
	[GLFW_KEY_K] = 75,
	[GLFW_KEY_L] = 76,
	[GLFW_KEY_M] = 77,
	[GLFW_KEY_N] = 78,
	[GLFW_KEY_O] = 79,
	[GLFW_KEY_P] = 80,
	[GLFW_KEY_Q] = 81,
	[GLFW_KEY_R] = 82,
	[GLFW_KEY_S] = 83,
	[GLFW_KEY_T] = 84,
	[GLFW_KEY_U] = 85,
	[GLFW_KEY_V] = 86,
	[GLFW_KEY_W] = 87,
	[GLFW_KEY_X] = 88,
	[GLFW_KEY_Y] = 89,
	[GLFW_KEY_Z] = 90,
	[GLFW_KEY_LEFT_BRACKET] = '{' /* [ */,
	[GLFW_KEY_BACKSLASH] = '|' /* \ */,
	[GLFW_KEY_RIGHT_BRACKET] = '}' /* ] */,
	[GLFW_KEY_GRAVE_ACCENT] = '~' /* ` */,
};

static int
transform_glfw_key(struct opengl_ui_ctx *ctx, int k)
{
	int rk;

	if ((ctx->keyboard.mod & mod_shift) && (k <= 255)) {
		rk = shifted_keys[k];
	} else if (k >= 'A' && k <= 'Z') {
		rk = k + 32;
	} else {
		switch (k) {
		case GLFW_KEY_UP:
			rk = skc_up;
			break;
		case GLFW_KEY_DOWN:
			rk = skc_down;
			break;
		case GLFW_KEY_LEFT:
			rk = skc_left;
			break;
		case GLFW_KEY_RIGHT:
			rk = skc_right;
			break;
		case GLFW_KEY_ENTER:
			rk = '\n';
			break;
		case GLFW_KEY_TAB:
			rk = '\t';
			break;
		case GLFW_KEY_F1:
			rk = skc_f1;
			break;
		case GLFW_KEY_F2:
			rk = skc_f2;
			break;
		case GLFW_KEY_F3:
			rk = skc_f3;
			break;
		case GLFW_KEY_F4:
			rk = skc_f4;
			break;
		case GLFW_KEY_F5:
			rk = skc_f5;
			break;
		case GLFW_KEY_F6:
			rk = skc_f6;
			break;
		case GLFW_KEY_F7:
			rk = skc_f7;
			break;
		case GLFW_KEY_F8:
			rk = skc_f8;
			break;
		case GLFW_KEY_F9:
			rk = skc_f9;
			break;
		case GLFW_KEY_F10:
			rk = skc_f10;
			break;
		case GLFW_KEY_F11:
			rk = skc_f11;
			break;
		case GLFW_KEY_F12:
			rk = skc_f12;
			break;
		default:
			rk = k;
			break;
		}
	}

	return rk;
}

static void
key_callback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
	struct opengl_ui_ctx *ctx = glfwGetWindowUserPointer(window);

	if (!cam.unlocked) {
		key = transform_glfw_key(ctx, key);
	}

	if (key < 0xff && key > 0) {
		if (action == GLFW_PRESS || action == GLFW_REPEAT) {
			ctx->keyboard.held[key] = 1;
		} else {
			ctx->keyboard.held[key] = 0;
		}
	} else if (key == GLFW_KEY_RIGHT_SHIFT || key == GLFW_KEY_LEFT_SHIFT) {
		if (action == GLFW_PRESS || action == GLFW_REPEAT) {
			ctx->keyboard.mod |= mod_shift;
		} else {
			ctx->keyboard.mod &= ~mod_shift;
		}
	} else if (key == GLFW_KEY_RIGHT_CONTROL || key == GLFW_KEY_LEFT_CONTROL) {
		if (action == GLFW_PRESS || action == GLFW_REPEAT) {
			ctx->keyboard.mod |= mod_ctrl;
		} else {
			ctx->keyboard.mod &= ~mod_ctrl;
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

void
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

void
constrain_cursor(struct opengl_ui_ctx *ctx, struct hiface *hf)
{
	if (hf->cursor.y <= 0) {
		hf->cursor.y = 1;
	} else if (hf->cursor.y >= ctx->ref.height) {
		hf->cursor.y = ctx->ref.height - 1;
	}

	if (hf->cursor.x <= 0) {
		hf->cursor.x = 1;
	} else if (hf->cursor.x >= ctx->ref.width) {
		hf->cursor.x = ctx->ref.width - 1;
	}

}

void
handle_gl_mouse(struct opengl_ui_ctx *ctx, struct hiface *hf)
{
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

	if (ctx->mouse.scroll != 0) {
		float mul = ctx->keyboard.mod & mod_shift ? 4.0 : 1.0;

		cam.pos[1] += floorf(ctx->mouse.scroll * SCROLL_SENS) * mul;

		ctx->mouse.scroll = 0;

		goto skip_mouse;
	} else if (ctx->keyboard.mod & mod_shift && ctx->mouse.old_buttons & mb_1
		   && !(ctx->mouse.buttons & mb_1)) {
		trigger_cmd(exec_action, hf);
		goto skip_mouse;
	} else if (cam.unlocked) {
		cam.yaw += ctx->mouse.dx * LOOK_SENS;
		cam.pitch += ctx->mouse.dy * LOOK_SENS;
		goto skip_mouse;
	}

	ctx->mouse.dx *= cam.pos[1] * 0.001;
	ctx->mouse.dy *= cam.pos[1] * 0.001;

	ctx->mouse.cursx += ctx->mouse.dx;
	ctx->mouse.cursy += ctx->mouse.dy;

	if (ctx->mouse.buttons & mb_1) {
		if (ctx->keyboard.mod & mod_shift) {
			override_num_arg(hf, fabs(floorf(ctx->mouse.dx)));
			if (ctx->mouse.dx > 0) {
				trigger_cmd(action_width_grow, hf);
			} else {
				trigger_cmd(action_width_shrink, hf);
			}

			override_num_arg(hf, fabs(floorf(ctx->mouse.dy)));
			if (ctx->mouse.dy > 0) {
				trigger_cmd(action_height_grow, hf);
			} else {
				trigger_cmd(action_height_shrink, hf);
			}
		} else {
			hf->view.x -= floor(ctx->mouse.cursx);
			hf->view.y -= floor(ctx->mouse.cursy);
			hf->cursor.x += floor(ctx->mouse.cursx);
			hf->cursor.y += floor(ctx->mouse.cursy);
			constrain_cursor(ctx, hf);
		}
	} else {
		hf->cursor.x += floor(ctx->mouse.cursx);
		hf->cursor.y += floor(ctx->mouse.cursy);
	}

	ctx->mouse.cursx -= floor(ctx->mouse.cursx);
	ctx->mouse.cursy -= floor(ctx->mouse.cursy);

	ctx->mouse.still = true;

skip_mouse:
	ctx->mouse.old_buttons = ctx->mouse.buttons;
}

static void
handle_flying_keys(struct opengl_ui_ctx *ctx, struct hiface *hf, size_t i)
{
	float speed = ctx->keyboard.mod & mod_shift ? 4.0f : 2.0f;

	vec4 v1;
	memcpy(v1, cam.tgt, sizeof(float) * 4);

	switch (i) {
	case 'w':
	case 'W':
		vec4_scale(v1, speed);
		vec4_sub(cam.pos, v1);
		break;
	case 's':
	case 'S':
		vec4_scale(v1, speed);
		vec4_add(cam.pos, v1);
		break;
	case 'a':
	case 'A':
		vec4_cross(v1, cam.up);
		vec4_normalize(v1);
		vec4_scale(v1, speed);
		vec4_add(cam.pos, v1);
		break;
	case 'd':
	case 'D':
		vec4_cross(v1, cam.up);
		vec4_normalize(v1);
		vec4_scale(v1, speed);
		vec4_sub(cam.pos, v1);
		break;
	}
}

void
handle_held_keys(struct opengl_ui_ctx *ctx, struct hiface *hf, struct keymap **km)
{
	size_t i;

	for (i = 0; i < 0xff; ++i) {
		if (!ctx->keyboard.held[i]) {
			continue;
		}

		if (ctx->keyboard.mod & mod_ctrl) {
			switch (i) {
			case 'i': case 'I':
				if ((wireframe = !wireframe)) {
					glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
				} else {
					glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
				}

				goto unhold_key;
			case 'u': case 'U':
				if (!(cam.unlocked = !cam.unlocked)) {
					cam.pitch = CAM_PITCH;
					cam.yaw = DEG_90;
				}

				goto unhold_key;
			case 'd': case 'D':
				ctx->debug_hud = !ctx->debug_hud;
				goto unhold_key;
			case 's': case 'S':
			{
				struct camera tmp = sun;
				sun = cam;
				cam = tmp;
			}
				sun.changed = true;
				cam.unlocked = true;

				goto unhold_key;
			case ' ':
			{
				float a = fabs(ctx->opts.cam_pitch_max - cam.pitch),
				      b = fabs(ctx->opts.cam_pitch_min - cam.pitch);

				ctx->cam_animation.pitch = a > b ?  1 : -1;
			}
				goto unhold_key;
			}
		}

		if (cam.unlocked) {
			handle_flying_keys(ctx, hf, i);
			continue;
		}

		ctx->last_key = i;

		if ((*km = handle_input(*km, i, hf)) == NULL) {
			*km = &hf->km[hf->im];
		}

unhold_key:
		ctx->keyboard.held[i] = 0;
	}
}

void
set_input_callbacks(struct GLFWwindow *window)
{
	glfwSetKeyCallback(window, key_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);
	glfwSetMouseButtonCallback(window, mouse_button_callback);

	cam.changed = true;
}
