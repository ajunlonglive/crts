#include "shared/opengl/window.h"
#include "shared/opengl/input.h"

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
transform_glfw_key(struct gl_input *in, int k)
{
	int rk;

	if ((in->keyboard.mod & mod_shift) && (k <= 255)) {
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
		case GLFW_KEY_BACKSPACE:
			rk = '\b';
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

	if (!in->keyboard.flying) {
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
