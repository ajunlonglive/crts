#ifndef CLIENT_UI_OPENGL_GLOBALS_H
#define CLIENT_UI_OPENGL_GLOBALS_H

#include <stdint.h>

#include "shared/math/linalg.h"

#define DEG_90 1.57f
#define FOV 0.47f
#define NEAR 0.1f
#define FAR 1000.0f

extern struct camera cam;

#define CHUNK_INDICES_LEN (512 * 3)
extern const uint16_t chunk_indices[CHUNK_INDICES_LEN];
#endif
