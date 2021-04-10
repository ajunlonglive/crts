#ifndef CLIENT_UI_OPENGL_COLORS_H
#define CLIENT_UI_OPENGL_COLORS_H

#include "shared/math/linalg.h"
#include "shared/sim/ent.h"

enum addl_ent_types {
	et_elf_friend = ent_type_count + 1,
	et_elf_foe,
	extended_ent_type_count,
};

struct colors {
	vec4 ent[extended_ent_type_count];
	vec4 tile[tile_count];
	vec4 sky;
};

extern struct colors colors;
#endif
