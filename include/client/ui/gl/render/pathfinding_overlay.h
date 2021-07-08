#ifndef CLIENT_UI_OPENGL_RENDER_PATHFINDING_OVERLAY_H
#define CLIENT_UI_OPENGL_RENDER_PATHFINDING_OVERLAY_H

#include "client/ui/gl/ui.h"

bool render_world_setup_pathfinding_overlay(void);
void render_pathfinding_overlay_setup_frame(struct client *cli, struct gl_ui_ctx *ctx);
void render_pathfinding_overlay(struct client *cli, struct gl_ui_ctx *ctx);
#endif
