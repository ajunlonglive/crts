#ifndef CLIENT_UI_OPENGL_GLOBALS_H
#define CLIENT_UI_OPENGL_GLOBALS_H

#include <stdint.h>

#include "shared/math/linalg.h"
#include "shared/math/geom.h"

#define DEG_90 1.57f
#define FOV 0.47f
#define NEAR 0.1f
#define FAR 1000.0f

#define CAM_PITCH_MIN D2R(25.0f)
#define CAM_PITCH_MAX D2R(89.9f)
#define CAM_YAW D2R(90.0f)

#define CAM_HEIGHT_MAX 400.0f
#define CAM_HEIGHT_MIN 0.0f

extern struct camera cam;
extern struct camera sun;

#define CHUNK_INDICES_LEN (512 * 3)
extern const uint32_t chunk_indices[CHUNK_INDICES_LEN];
#endif
