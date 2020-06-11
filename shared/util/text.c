#include <stdlib.h>
#include <string.h>

#include "shared/util/text.h"

#define CSIZE 1024

void
each_line(FILE *infile, void *ctx, each_line_callback cb)
{
	size_t read, i, lo = 0;
	char ibuf[CSIZE] = { 0 }, *line, *b;

	while ((read = fread(&ibuf[lo], sizeof(char), CSIZE - (1 + lo), infile))) {
		read += lo;
		line = ibuf;
		i = 0;

		while ((b = strchr(line, '\n'))) {
			i = b - ibuf;
			*b = '\0';

			cb(ctx, line, b - line);

			line = b + 1;
		}

		if (i < read) {
			lo = read - i - 1;
			memmove(ibuf, line, lo);
		} else {
			lo = 0;
		}
	}
}
