#ifndef CLIENT_UI_OPENGL_UI_H
#define CLIENT_UI_OPENGL_UI_H
#include <glad/gl.h>
#include <GLFW/glfw3.h>

#include "client/hiface.h"
#include "client/input/keymap.h"
#include "shared/math/linalg.h"

enum mouse_buttons {
	mb_1 = 1 << 0,
	mb_2 = 1 << 1,
	mb_3 = 1 << 2,
	mb_4 = 1 << 3,
	mb_5 = 1 << 4,
	mb_6 = 1 << 5,
	mb_7 = 1 << 6,
	mb_8 = 1 << 7,
};

enum modifier_types {
	mod_shift = 1 << 0,
};

struct opengl_ui_ctx {
	struct rectangle ref;
	int width, height;
	bool resized;
	struct hash *echash;
	mat4 mproj;
	GLFWwindow* window;
	struct {
		double lx, ly, x, y, dx, dy, scroll;
		double cursx, cursy;
		bool still, init;
		uint8_t buttons;
	} mouse;
	struct {
		uint8_t held[0xff];
		uint8_t mod;
	} keyboard;

};

struct opengl_ui_ctx *opengl_ui_init(char *graphics_path);
void opengl_ui_render(struct opengl_ui_ctx *nc, struct hiface *hf);
void opengl_ui_handle_input(struct opengl_ui_ctx *ctx, struct keymap **km,
	struct hiface *hf);
struct rectangle opengl_ui_viewport(struct opengl_ui_ctx *nc);
void opengl_ui_deinit(struct opengl_ui_ctx *ctx);
#endif
