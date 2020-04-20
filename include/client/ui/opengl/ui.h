#ifndef CLIENT_UI_OPENGL_UI_H
#define CLIENT_UI_OPENGL_UI_H
#include "glad/glad.h"
#include <GLFW/glfw3.h>

#include "client/hiface.h"
#include "client/input/keymap.h"

struct opengl_ui_ctx {
	GLFWwindow* window;
	uint32_t prog_id, vao, vbo;
	struct {
		uint32_t mod, view, proj, clr;
	} uni;
};

struct opengl_ui_ctx *opengl_ui_init(char *graphics_path);
void opengl_ui_render(struct opengl_ui_ctx *nc, struct hiface *hf);
void opengl_ui_handle_input(struct keymap **km, struct hiface *hf);
struct rectangle opengl_ui_viewport(struct opengl_ui_ctx *nc);
void opengl_ui_deinit(void);
#endif
