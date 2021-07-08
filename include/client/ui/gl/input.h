#ifndef CLIENT_UI_GL_INPUT_H
#define CLIENT_UI_GL_INPUT_H

#include "client/ui/gl/ui.h"

void handle_gl_mouse(struct gl_ui_ctx *ctx, struct client *cli);
void handle_held_keys(struct gl_ui_ctx *ctx);
void set_input_callbacks(struct gl_ui_ctx *ctx);
void gl_ui_handle_input(struct gl_ui_ctx *ctx, struct client *cli);
#endif
