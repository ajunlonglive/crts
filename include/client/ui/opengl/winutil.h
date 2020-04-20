#ifndef UTIL_H
#define UTIL_H
#include <stdbool.h>
#include <stdio.h>

#include "client/ui/opengl/ui.h"

struct shader_src {
	const char *path;
	GLenum type;
	uint32_t id;
};

GLFWwindow * init_window(void);
bool link_shaders(struct shader_src *ss, uint32_t *program);
#endif
