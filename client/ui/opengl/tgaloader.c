#include "posix.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "client/ui/opengl/tgaloader.h"
#include "shared/util/log.h"

#define CHUNK_SIZE 512

uint8_t *
load_tga(const char *path)
{
	FILE *f;
	size_t len, b, i = 0;

	if (!(f = fopen(path, "r"))) {
		L("error opening font atlas '%s': %s", path, strerror(errno));
		return NULL;
	}

	fseek(f, 0, SEEK_END);
	len = ftell(f);
	rewind(f);

	/* TODO: parse the header */
	fseek(f, 18, SEEK_SET);

	uint8_t *buf = malloc(sizeof(char) * (len - 18));

	while ((b = fread(&buf[i], 1, CHUNK_SIZE, f))) {
		if (i == 0) {
			L("%d, %d, %d, %d", buf[0], buf[1], buf[2], buf[3]);
		}
		i += b;
	}

	L("read %ld bytes into buffer(%ld)", i, len);

	return buf;
}
