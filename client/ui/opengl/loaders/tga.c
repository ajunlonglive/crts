#include "posix.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "client/ui/opengl/loaders/tga.h"
#include "shared/util/assets.h"
#include "shared/util/log.h"


struct tga_header {
	uint8_t idlength;        // 0
	uint8_t colormaptype;    // 1
	uint8_t datatypecode;    // 2
	uint16_t colormaporigin; // 3,4
	uint16_t colormaplength; // 5,6
	uint8_t colormapdepth;   // 7
	uint16_t x_origin;       // 8,9
	uint16_t y_origin;       // 10,11
	uint16_t width;          // 12,13
	uint16_t height;         // 14,15
	uint8_t bitsperpixel;    // 16
	uint8_t imagedescriptor; // 17
};

const uint8_t *
load_tga(const char *path, uint16_t *width, uint16_t *height, uint8_t *bits)
{
	struct file_data *fd;
	if (!(fd = asset(path))) {
		return NULL;
	}

	struct tga_header hdr = {
		.idlength = fd->data[0],
		.colormaptype = fd->data[1],
		.datatypecode = fd->data[2],
		.colormaporigin = *(uint16_t *)&fd->data[3],
		.colormaplength = *(uint16_t *)&fd->data[5],
		.colormapdepth = fd->data[7],
		.x_origin = *(uint16_t *)&fd->data[8],
		.y_origin = *(uint16_t *)&fd->data[10],
		.width = *(uint16_t *)&fd->data[12],
		.height = *(uint16_t *)&fd->data[14],
		.bitsperpixel = fd->data[16],
		.imagedescriptor = fd->data[17],
	};

	*width = hdr.width;
	*height = hdr.height;
	*bits = hdr.bitsperpixel;

	return &fd->data[18];
}
