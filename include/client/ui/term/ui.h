#ifndef CLIENT_UI_TERM_UI_H
#define CLIENT_UI_TERM_UI_H

#include "client/client.h"

struct term_ui_ctx {
	struct { uint32_t world, info_l, info_r; } wins;
};

bool term_ui_init(struct term_ui_ctx *ctx);
void term_ui_render(struct term_ui_ctx *ctx, struct client *cli);
void term_ui_handle_input(struct term_ui_ctx *ctx, struct client *cli);
void term_ui_deinit(void);
const struct rectangle *term_ui_viewport(struct term_ui_ctx *nc);
#endif
