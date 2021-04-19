#ifndef TERRAGEN_OPENGL_UI_H
#define TERRAGEN_OPENGL_UI_H

#include "shared/opengl/menu.h"
#include "shared/opengl/window.h"
#include "terragen/gen/gen.h"

struct ui_ctx {
	struct gl_win win;
	struct menu_ctx menu_ctx;
	float mousex, mousey;
	uint32_t mb_pressed, mb_released;
	struct terragen_ctx ctx;
	terragen_opts opts;
	float heightmap_opacity;
	bool dim_changed;
	bool write_file;
};

void genworld_interactive(terragen_opts opts, const char *outfile);
#endif
