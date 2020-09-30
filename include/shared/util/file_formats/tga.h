#ifndef SHARED_UTIL_FILE_FORMATS_TGA_H
#define SHARED_UTIL_FILE_FORMATS_TGA_H

#include <stdint.h>
#include <stdio.h>

struct tga_header {
	uint16_t colormaplength; // 5,6
	uint16_t colormaporigin; // 3,4
	uint16_t height;         // 14,15
	uint16_t width;          // 12,13
	uint16_t x_origin;       // 8,9
	uint16_t y_origin;       // 10,11
	uint8_t bitsperpixel;    // 16
	uint8_t colormapdepth;   // 7
	uint8_t colormaptype;    // 1
	uint8_t datatypecode;    // 2
	uint8_t idlength;        // 0
	uint8_t imagedescriptor; // 17
};

void write_tga_hdr(FILE *f, uint32_t height, uint32_t width);
const uint8_t *parse_tga_hdr(const uint8_t *data, uint16_t *width, uint16_t *height, uint8_t *bits);
#endif
