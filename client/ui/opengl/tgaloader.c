#include "posix.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "client/ui/opengl/tgaloader.h"
#include "shared/util/assets.h"
#include "shared/util/log.h"

uint8_t *
load_tga(const char *path)
{
	struct file_data *fd;

	if (!(fd = asset(path))) {
		return NULL;
	}

	/* TODO: parse the header */
	return &fd->data[18];
}
