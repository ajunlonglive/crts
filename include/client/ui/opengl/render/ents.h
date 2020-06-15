#ifndef CLIENT_UI_OPENGL_RENDER_ENTS
#define CLIENT_UI_OPENGL_RENDER_ENTS

#include "client/ui/opengl/ui.h"

bool render_world_setup_ents(void);
void render_ents(struct hiface *hf, struct opengl_ui_ctx *ctx, mat4 mview);
#endif
