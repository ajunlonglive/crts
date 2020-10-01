#ifndef SHARED_UTIL_TEXT_H
#define SHARED_UTIL_TEXT_H

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>

#include "shared/util/assets.h"

bool is_whitespace(char c);

typedef void ((*each_line_callback)(void *ctx, char *line, size_t len));

void each_line(struct file_data *fd, void *ctx, each_line_callback cb);

typedef bool ((*parse_optstring_cb)(void *ctx, const char *k, const char *v));

bool parse_optstring(char *s, void *ctx, parse_optstring_cb cb);
#endif
