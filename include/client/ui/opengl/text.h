#ifndef CLIENT_UI_OPENGL_TEXT_H
#define CLIENT_UI_OPENGL_TEXT_H

#include <stddef.h>

#include "shared/math/linalg.h"

void text_init(void);

size_t gl_printf(float x, float y, const char *fmt, ...);
size_t gl_write_string(float x, float y, float scale, vec4 clr, const char *str);
void update_text_viewport(int width, int height);
void text_setup_render(void);
#endif
