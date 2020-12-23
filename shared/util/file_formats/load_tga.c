#include "posix.h"

#include "shared/util/assets.h"
#include "shared/util/file_formats/load_tga.h"
#include "shared/util/file_formats/tga.h"

const uint8_t *
load_tga(const char *path, uint16_t *width, uint16_t *height, uint8_t *bits)
{
	struct file_data *fd;
	if (!(fd = asset(path))) {
		return NULL;
	}

	return parse_tga_hdr(fd->data, width, height, bits);
}
