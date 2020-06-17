#ifndef SHARED_UTIL_TEXT_H
#define SHARED_UTIL_TEXT_H

#include <stddef.h>
#include <stdio.h>

#include "shared/util/assets.h"

typedef void ((*each_line_callback)(void *ctx, char *line, size_t len));

void each_line(struct file_data *fd, void *ctx, each_line_callback cb);
#endif
