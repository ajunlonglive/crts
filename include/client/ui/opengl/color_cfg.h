#ifndef CLIENT_UI_OPENGL_COLOR_CFG_H
#define CLIENT_UI_OPENGL_COLOR_CFG_H

#include "client/ui/graphics.h"
#include "shared/math/linalg.h"

struct colors_t {
	struct vec4 ent[extended_ent_type_count];
	struct vec4 tile[tile_count];
};

extern struct colors_t colors;

bool color_cfg(char *file);
#endif
