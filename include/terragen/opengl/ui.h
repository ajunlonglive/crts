#ifndef TERRAGEN_OPENGL_UI_H
#define TERRAGEN_OPENGL_UI_H

#include "terragen/gen/gen.h"
#include "shared/opengl/window.h"

struct ui_ctx {
	struct GLFWwindow *glfw_win;
	struct gl_win win;
	float mousex, mousey;
	float text_scale;
	uint32_t mb_pressed, mb_released;
	struct terragen_ctx ctx;
	terragen_opts opts;
	float heightmap_opacity;
};

void genworld_interactive(terragen_opts opts);
#endif
