#ifndef CLIENT_UI_OPENGL_RENDER_ENTS
#define CLIENT_UI_OPENGL_RENDER_ENTS

#include "client/ui/opengl/ui.h"

bool render_world_setup_ents(void);
void render_ents_setup_frame(struct client *cli, struct opengl_ui_ctx *ctx);
void render_ents(struct client *cli, struct opengl_ui_ctx *ctx);
#endif
