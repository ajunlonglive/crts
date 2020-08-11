#ifndef TERRAGEN_OPENGL_UI_H
#define TERRAGEN_OPENGL_UI_H

#include "terragen/gen/gen.h"
#include "shared/opengl/window.h"

struct ui_ctx {
	struct GLFWwindow *glfw_win;
	struct gl_win win;
	float mousex, mousey;
	float text_scale;
	uint32_t mb;
	struct terragen_ctx ctx;
	struct terragen_opts *opts;
};

void genworld_interactive(struct terragen_opts *opts);
#endif
