#ifndef CLIENT_UI_OPENGL_UTIL_H
#define CLIENT_UI_OPENGL_UTIL_H

#include <stdint.h>

#include "client/ui/opengl/ui.h"

uint32_t fb_attach_color(uint32_t w, uint32_t h);
uint32_t fb_attach_db(uint32_t w, uint32_t h);
uint32_t fb_attach_dtex(uint32_t w, uint32_t h);
int32_t load_tex(char *asset, GLenum wrap, GLenum filter);
#endif
