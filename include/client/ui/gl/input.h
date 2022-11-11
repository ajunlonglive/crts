#ifndef CLIENT_UI_GL_INPUT_H
#define CLIENT_UI_GL_INPUT_H

#include "client/ui/gl/ui.h"

void trace_cursor_to_world(struct gl_ui_ctx *ctx, struct client *cli);
void set_input_callbacks(struct gl_ui_ctx *ctx);
void register_input_cfg_data(void);
void gl_ui_handle_input(struct gl_ui_ctx *ctx, struct client *cli);
#endif
