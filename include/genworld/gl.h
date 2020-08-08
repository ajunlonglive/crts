#ifndef GENWORLD_GL_H
#define GENWORLD_GL_H

#include "genworld/gen.h"
#include "shared/opengl/window.h"

struct ui_ctx {
	struct GLFWwindow *glfw_win;
	struct gl_win win;
	float mousex, mousey;
	uint32_t mb;
	const struct gen_terrain_ctx gt;
	struct worldgen_opts *opts;
};

void genworld_interactive(struct worldgen_opts *opts);
#endif
