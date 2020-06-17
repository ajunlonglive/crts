#ifndef SHARED_UTIL_INIH_H
#define SHARED_UTIL_INIH_H

#include <stdbool.h>
#include <stdint.h>

#include "shared/util/assets.h"

typedef bool ((*inihcb)(void *ctx, const char *sect, const char *k,
			const char *v, uint32_t line));

bool ini_parse(struct file_data *fd, inihcb cb, void *ctx);
#endif
