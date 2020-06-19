#ifndef CLIENT_UI_OPENGL_TEXT_H
#define CLIENT_UI_OPENGL_TEXT_H

#include <stddef.h>

#include "client/ui/opengl/ui.h"
#include "shared/math/linalg.h"

bool render_text_setup(float scale);

size_t gl_printf(float x, float y, const char *fmt, ...);
size_t gl_write_string(float x, float y, float scale, vec4 clr, const char *str);
void text_setup_render(struct opengl_ui_ctx *ctx);
void screen_coords_to_text_coords(float x, float y, float *sx, float *sy);
size_t gl_write_string_centered(float x, float y, float scale, vec4 clr,
	const char *str);
#endif
