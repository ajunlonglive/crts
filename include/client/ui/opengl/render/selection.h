#ifndef CLIENT_UI_OPENGL_RENDER_SELECTION
#define CLIENT_UI_OPENGL_RENDER_SELECTION
#include "client/ui/opengl/ui.h"

bool render_world_setup_selection(void);
void render_selection(struct hiface *hf, struct opengl_ui_ctx *ctx,
	struct hdarr *cms);
#endif
