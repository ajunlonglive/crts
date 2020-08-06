#ifndef CLIENT_UI_OPENGL_LOADERS_SHADER_H
#define CLIENT_UI_OPENGL_LOADERS_SHADER_H

#include <glad/gl.h>
#include <stdbool.h>
#include <stdint.h>

struct shader_src {
	const char *path;
	uint32_t type;
	GLenum id;
};

bool link_shaders(struct shader_src *ss, uint32_t *program);
#endif
