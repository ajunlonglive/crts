#ifndef CLIENT_UI_OPENGL_LOADERS_SHADER_H
#define CLIENT_UI_OPENGL_LOADERS_SHADER_H

#include "client/ui/opengl/ui.h"

struct shader_src {
	const char *path;
	GLenum type;
	uint32_t id;
};

bool link_shaders(struct shader_src *ss, uint32_t *program);
#endif
