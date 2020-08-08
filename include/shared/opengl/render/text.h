#ifndef CLIENT_UI_OPENGL_TEXT_H
#define CLIENT_UI_OPENGL_TEXT_H

#include <stddef.h>

#include "shared/math/linalg.h"
#include "shared/opengl/window.h"
#include "shared/types/geom.h"

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
void render_text(struct gl_win *win);

typedef void ((*interactive_text_cb)(void *ctx, bool hover, float x, float y, const char *str));

void gl_mprintf(float sx, float sy, enum text_anchor anch, const struct pointf *c,
	void *ctx, interactive_text_cb cb, const char *fmt, ...);
#endif
