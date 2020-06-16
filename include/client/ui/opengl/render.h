#ifndef CLIENT_UI_OPENGL_RENDER_H
#define CLIENT_UI_OPENGL_RENDER_H

#include "client/ui/opengl/ui.h"

bool opengl_ui_render_setup(void);
void opengl_ui_render_teardown(void);
void opengl_ui_render(struct opengl_ui_ctx *ctx, struct hiface *hf);
#endif
