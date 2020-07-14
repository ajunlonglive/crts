#ifndef CLIENT_UI_OPENGL_TGALOADER_H
#define CLIENT_UI_OPENGL_TGALOADER_H
#include <stdint.h>

const uint8_t *load_tga(const char *path, uint16_t *width, uint16_t *height,
	uint8_t *bits);
#endif
