#ifndef CLIENT_UI_OPENGL_RENDER_SUN_H
#define CLIENT_UI_OPENGL_RENDER_SUN_H

#include "client/ui/opengl/ui.h"

bool render_world_setup_sun(void);
void render_sun_setup_frame(struct opengl_ui_ctx *ctx);
void render_sun(struct opengl_ui_ctx *ctx);
#endif
