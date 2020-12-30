#ifndef CLIENT_UI_OPENGL_RENDER_SELECTION
#define CLIENT_UI_OPENGL_RENDER_SELECTION
#include "client/ui/opengl/ui.h"

bool render_world_setup_selection(void);
void render_selection_setup_frame(struct client *cli, struct opengl_ui_ctx *ctx,
	struct hdarr *cms);
void render_selection(struct client *cli, struct opengl_ui_ctx *ctx,
	struct hdarr *cms);
#endif
