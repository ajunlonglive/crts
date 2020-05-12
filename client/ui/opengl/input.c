#include <math.h>
#include <string.h>

#include "client/hiface.h"
#include "client/ui/opengl/globals.h"
#include "client/ui/opengl/input.h"
#include "client/ui/opengl/ui.h"
#include "shared/util/log.h"

#define LOOK_SENS 0.0026646259971648;

enum modifier_types {
	mod_shift = 1 << 0,
};

static struct {
	uint8_t held[0xff];
	uint8_t mod;
} keyboard = { 0 };

static struct {
	double lx, ly, x, y;
	bool still, init;
} mouse = { 0 };

static void
key_callback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
	if (key < 0xff && key > 0) {
		if (action == GLFW_PRESS || action == GLFW_REPEAT) {
			keyboard.held[key] = 1;
		} else {
			keyboard.held[key] = 0;
		}
	} else if (key == GLFW_KEY_LEFT_SHIFT) {
		if (action == GLFW_PRESS || action == GLFW_REPEAT) {
			keyboard.mod |= mod_shift;
		} else {
			keyboard.mod &= ~mod_shift;
		}
	}
}

static void
mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	mouse.x = xpos;
	mouse.y = ypos;

	mouse.still = false;
}

static void
handle_flying_mouse(double dx, double dy)
{
	cam.yaw   += dx * LOOK_SENS;
	cam.pitch += dy * LOOK_SENS;

	if (cam.pitch > DEG_90) {
		cam.pitch = DEG_90;
	} else if (cam.pitch < -DEG_90) {
		cam.pitch = -DEG_90;
	}

	cam.changed = true;
}

void
handle_gl_mouse(struct hiface *hf)
{
	double dx, dy;

	if (mouse.still) {
		return;
	} else {
		dx = mouse.x - mouse.lx;
		dy = mouse.y - mouse.ly;

		mouse.lx = mouse.x;
		mouse.ly = mouse.y;

		if (!mouse.init) {
			mouse.init = true;
			return;
		}
	}

	handle_flying_mouse(dx, dy);
	mouse.still = true;
}

void
handle_flying_keys(struct hiface *hf, size_t i)
{
	float speed = keyboard.mod & mod_shift ? 4.0f : 2.0f;

	vec4 v1;
	memcpy(v1, cam.tgt, sizeof(float) * 4);

	switch (i) {
	case GLFW_KEY_K:
		vec4_scale(v1, speed);
		vec4_sub(cam.pos, v1);
		break;
	case GLFW_KEY_J:
		vec4_scale(v1, speed);
		vec4_add(cam.pos, v1);
		break;
	case GLFW_KEY_H:
		vec4_cross(v1, cam.up);
		vec4_normalize(v1);
		vec4_scale(v1, speed);
		vec4_add(cam.pos, v1);
		break;
	case GLFW_KEY_L:
		vec4_cross(v1, cam.up);
		vec4_normalize(v1);
		vec4_scale(v1, speed);
		vec4_sub(cam.pos, v1);
		break;
	}

	cam.changed = true;
}

void
handle_held_keys(struct hiface *hf)
{
	size_t i;

	for (i = 0; i < 0xff; ++i) {
		if (!keyboard.held[i]) {
			continue;
		}

		switch (i) {
		case GLFW_KEY_V:
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			break;
		case GLFW_KEY_Z:
			cam.yaw -= 0.1;
			break;
		case GLFW_KEY_C:
			cam.yaw += 0.1;
			break;
		case GLFW_KEY_Q:
			hf->sim->run = false;
			break;
		default:
			handle_flying_keys(hf, i);
		}
	}
}

void
set_input_callbacks(struct GLFWwindow *window)
{
	glfwSetKeyCallback(window, key_callback);
	glfwSetCursorPosCallback(window, mouse_callback);

	cam.changed = true;
}
