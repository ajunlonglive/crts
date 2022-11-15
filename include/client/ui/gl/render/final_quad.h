#ifndef CLIENT_UI_OPENGL_RENDER_FINAL_QUAD_H
#define CLIENT_UI_OPENGL_RENDER_FINAL_QUAD_H

#include "client/ui/gl/ui.h"

struct final_quad_fx {
	uint32_t fb, fb_depth, fb_tex;
};

bool render_world_setup_final_quad(struct gl_ui_ctx *ctx, struct final_quad_fx *fqfx);
void render_final_quad_setup_frame(struct gl_ui_ctx *ctx, struct final_quad_fx *fqfx);
void render_final_quad(struct client *cli, struct gl_ui_ctx *ctx, struct final_quad_fx *fqfx);
#endif
