#ifndef CLIENT_UI_OPENGL_RENDER_WORLD_H
#define CLIENT_UI_OPENGL_RENDER_WORLD_H
#include <stdbool.h>

#include "client/hiface.h"
#include "client/ui/opengl/ui.h"

bool render_world_setup(char *);
void render_world_teardown(void);
void update_world_viewport(mat4 mproj);
void render_world(struct opengl_ui_ctx *ctx, struct hiface *hf);
#endif
