#include "posix.h"

#include <stdlib.h>
#include <string.h>

#include "shared/util/assets.h"
#include "shared/util/log.h"
#include "shared/util/text.h"

void
each_line(struct file_data *fd, void *ctx, each_line_callback cb)
{
	char *line, *b;

	line = (char *)fd->data;

	while ((b = strchr(line, '\n'))) {
		*b = '\0';

		cb(ctx, line, b - line);

		line = b + 1;

		if ((size_t)((uint8_t *)line - fd->data) >= fd->len) {
			break;
		}
	}
}
