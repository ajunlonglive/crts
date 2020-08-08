#ifndef GENWORLD_RENDER_TERRAIN_H
#define GENWORLD_RENDER_TERRAIN_H
#include "genworld/gl.h"

bool render_terrain_init(struct ui_ctx *ctx);
void render_terrain_setup(struct ui_ctx *ctx);
void render_terrain(struct ui_ctx *ctx);
#endif
