#ifndef SHARED_UTIL_FILE_FORMATS_LOAD_TGA_H
#define SHARED_UTIL_FILE_FORMATS_LOAD_TGA_H
#include <stdint.h>

const uint8_t *load_tga(const char *path, uint16_t *width, uint16_t *height,
	uint8_t *bits);
#endif
