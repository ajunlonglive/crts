#ifndef CLIENT_UI_OPENGL_RENDER_H
#define CLIENT_UI_OPENGL_RENDER_H

#include "client/opts.h"
#include "client/ui/opengl/ui.h"

enum opengl_render_steps {
	opengl_render_step_ents        = 1 << 0,
	opengl_render_step_selection   = 1 << 1,
	opengl_render_step_chunks      = 1 << 2,
	opengl_render_step_shadows     = 1 << 3,
	opengl_render_step_reflections = 1 << 4,
};

bool opengl_ui_render_setup(struct opengl_ui_ctx *ctx);
void opengl_ui_render_teardown(void);
#endif
