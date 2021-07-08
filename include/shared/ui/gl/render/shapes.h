#ifndef SHARED_OPENGL_RENDER_SHAPES_H
#define SHARED_OPENGL_RENDER_SHAPES_H

#include <stdint.h>

#include "shared/math/linalg.h"

bool render_shapes_setup(void);
uint32_t render_shapes_add_rect(float x, float y, float h, float w, vec4 clr);
void render_shapes_resize(uint32_t i, float h, float w);
void render_shapes_clear(void);
void render_shapes_update_proj(mat4 proj);
void render_shapes(void);
#endif
