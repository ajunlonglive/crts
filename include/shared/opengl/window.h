#ifndef CLIENT_UI_OPENGL_WINDOW_H
#define CLIENT_UI_OPENGL_WINDOW_H
#include <stdbool.h>

#include <glad/gl.h>
#include <GLFW/glfw3.h>

struct gl_win {
	GLFWwindow *win;
	uint32_t px_height, px_width, sc_height, sc_width;
	bool resized;
};

bool init_window(struct gl_win *win);
#endif
