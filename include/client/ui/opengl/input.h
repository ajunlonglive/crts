#ifndef CLIENT_UI_OPENGL_INPUT_H
#define CLIENT_UI_OPENGL_INPUT_H

#include "client/input/keymap.h"
#include "client/ui/opengl/ui.h"

void handle_gl_mouse(struct opengl_ui_ctx *ctx, struct hiface *hf);
void handle_held_keys(struct opengl_ui_ctx *ctx);
void set_input_callbacks(struct GLFWwindow *window);
void constrain_cursor(struct opengl_ui_ctx *ctx, struct point *curs);
#endif
