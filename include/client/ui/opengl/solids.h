#ifndef CLIENT_UI_OPENGL_SOLIDS_H
#define CLIENT_UI_OPENGL_SOLIDS_H

#include <stddef.h>
#include <stdint.h>

struct solid {
	size_t len;
	float verts[0xff];
};

extern const struct solid solid_cube;

#define CHUNK_INDICES_LEN (512 * 3)
extern const uint16_t chunk_indices[CHUNK_INDICES_LEN];
#endif
