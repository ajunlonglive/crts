#include "posix.h"

#include "shared/util/file_formats/tga.h"

const uint8_t *
parse_tga_hdr(const uint8_t *data, uint16_t *width, uint16_t *height, uint8_t *bits)
{
	struct tga_header hdr = {
		.idlength = data[0],
		.colormaptype = data[1],
		.datatypecode = data[2],
		.colormaporigin = *(uint16_t *)&data[3],
		.colormaplength = *(uint16_t *)&data[5],
		.colormapdepth = data[7],
		.x_origin = *(uint16_t *)&data[8],
		.y_origin = *(uint16_t *)&data[10],
		.width = *(uint16_t *)&data[12],
		.height = *(uint16_t *)&data[14],
		.bitsperpixel = data[16],
		.imagedescriptor = data[17],
	};

	*width = hdr.width;
	*height = hdr.height;
	*bits = hdr.bitsperpixel;

	return &data[18];
}

void
write_tga_hdr(FILE *f, uint32_t height, uint32_t width)
{
	uint8_t hdr[18] = { 0 };

	hdr[2]  = 2;
	hdr[12] = 255 & height;
	hdr[13] = 255 & (height >> 8);
	hdr[14] = 255 & width;
	hdr[15] = 255 & (width >> 8);
	hdr[16] = 32;
	hdr[17] = 32;

	fwrite(hdr, 1, 18, f);
}
