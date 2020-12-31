#ifndef CLIENT_UI_OPENGL_RENDER_H
#define CLIENT_UI_OPENGL_RENDER_H

#include "client/opts.h"
#include "client/ui/opengl/ui.h"

bool opengl_ui_render_setup(struct opengl_ui_ctx *ctx);
void opengl_ui_render_teardown(void);
#endif
