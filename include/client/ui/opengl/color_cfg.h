#ifndef CLIENT_UI_OPENGL_COLOR_CFG_H
#define CLIENT_UI_OPENGL_COLOR_CFG_H

#include "client/ui/graphics.h"
#include "shared/math/linalg.h"

struct colors_t {
	vec4 ent[extended_ent_type_count];
	vec4 tile[tile_count];
};

extern struct colors_t colors;

bool color_cfg(void);
#endif
