#ifndef CLIENT_UI_OPENGL_RENDER_SHADOWS_H
#define CLIENT_UI_OPENGL_RENDER_SHADOWS_H

#include "client/ui/opengl/ui.h"

struct shadow_map {
	uint32_t dim;
	uint32_t depth_map_fb, depth_map_tex;
};

bool render_world_setup_shadows(struct shadow_map *sm);
#endif
