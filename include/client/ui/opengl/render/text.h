#ifndef CLIENT_UI_OPENGL_TEXT_H
#define CLIENT_UI_OPENGL_TEXT_H

#include <stddef.h>

#include "client/ui/opengl/ui.h"
#include "shared/math/linalg.h"

bool render_text_setup(float scale);

enum text_anchor {
	ta_left,
	ta_right,
};

void gl_write_char(float x, float y, vec4 clr, char c);
size_t gl_write_string(float x, float y, float scale, vec4 clr, const char *str);
size_t gl_printf(float x, float y, enum text_anchor anch, const char *fmt, ...);
size_t gl_write_string_centered(float x, float y, float scale, vec4 clr,
	const char *str);

void screen_coords_to_text_coords(float x, float y, float *sx, float *sy);
void render_text_clear(void);
void render_text_commit(void);
void render_text(struct opengl_ui_ctx *ctx);
#endif
