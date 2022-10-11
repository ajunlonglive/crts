#ifndef CLIENT_UI_OPENGL_COLORS_H
#define CLIENT_UI_OPENGL_COLORS_H

#include "shared/math/linalg.h"
#include "shared/sim/ent.h"

struct colors {
	vec4 ent[ent_type_count];
	vec4 tile[tile_count];
	vec4 sky;
};

extern struct colors colors;
#endif
