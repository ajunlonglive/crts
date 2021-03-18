#ifndef SHARED_OPENGL_RENDER_SHAPES_H
#define SHARED_OPENGL_RENDER_SHAPES_H

#include "shared/math/linalg.h"
#include "shared/opengl/window.h"

bool render_shapes_setup(void);
void render_shapes_add_rect(float x, float y, float h, float w, vec4 clr);
void render_shapes_clear(void);
void render_shapes(struct gl_win *win, mat4 proj);
#endif
