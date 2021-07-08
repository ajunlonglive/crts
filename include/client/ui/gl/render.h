#ifndef CLIENT_UI_OPENGL_RENDER_H
#define CLIENT_UI_OPENGL_RENDER_H

#include "client/opts.h"
#include "client/ui/gl/ui.h"

enum gl_render_steps {
	gl_render_step_ents        = 1 << 0,
	gl_render_step_selection   = 1 << 1,
	gl_render_step_chunks      = 1 << 2,
	gl_render_step_shadows     = 1 << 3,
	gl_render_step_reflections = 1 << 4,
};

bool gl_ui_render_setup(struct gl_ui_ctx *ctx);
void gl_ui_render_teardown(void);
#endif
