#ifndef CLIENT_UI_OPENGL_INPUT_H
#define CLIENT_UI_OPENGL_INPUT_H

#include "client/input/keymap.h"
#include "client/ui/opengl/ui.h"

struct GLFWwindow;
struct hiface;

void handle_gl_mouse(struct opengl_ui_ctx *ctx, struct hiface *hf);
void handle_held_keys(struct hiface *hf, struct keymap **km);
void set_input_callbacks(struct GLFWwindow *window);
#endif
