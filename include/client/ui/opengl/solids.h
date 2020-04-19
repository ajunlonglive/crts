#ifndef CLIENT_UI_OPENGL_SOLIDS_H
#define CLIENT_UI_OPENGL_SOLIDS_H

#include <stddef.h>

struct solid {
	size_t len;
	float verts[0xff];
};

extern const struct solid solid_cube;
#endif
