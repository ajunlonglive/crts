#ifndef TERRAGEN_OPENGL_RENDER_MESH_H
#define TERRAGEN_OPENGL_RENDER_MESH_H

#include "terragen/opengl/ui.h"

bool render_mesh_setup(struct ui_ctx *ctx);
void render_mesh_setup_frame(struct ui_ctx *ctx);
void render_mesh(struct ui_ctx *ctx);
#endif
