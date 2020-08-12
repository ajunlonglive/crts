#ifndef TERRAGEN_OPENGL_RENDER_PIXELS_H
#define TERRAGEN_OPENGL_RENDER_PIXELS_H

#include "terragen/opengl/ui.h"

bool render_pixels_setup(struct ui_ctx *ctx);
void render_pixels_setup_frame(struct ui_ctx *ctx);
void render_pixels(struct ui_ctx *ctx);
#endif
