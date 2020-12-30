#ifndef CLIENT_UI_OPENGL_HUD_H
#define CLIENT_UI_OPENGL_HUD_H

#include "client/ui/opengl/ui.h"

void render_hud(struct opengl_ui_ctx *ctx, struct client *cli);
void render_debug_hud(struct opengl_ui_ctx *ctx, struct client *cli);
#endif
