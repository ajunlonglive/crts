#include "posix.h"

#include <stddef.h>
#include <string.h>
#include <unistd.h>

#include "client/cfg/common.h"
#include "client/cfg/graphics.h"
#include "client/cfg/keymap.h"
#include "shared/util/log.h"

int32_t
cfg_string_lookup(const char *str, struct lookup_table *tbl)
{
	size_t i;

	for (i = 0; i < LOOKUP_TBL_LEN; ++i) {
		if (tbl->e[i].str == NULL) {
			break;
		} else if (strcmp(tbl->e[i].str, str) == 0) {
			return tbl->e[i].t;
		}
	}

	return -1;
}

bool
parse_cfg_file(const char *filename, void *ctx, inihcb handler)
{
	struct file_data *fd;

	if (!(fd = asset(filename))) {
		return false;
	}

	if (!ini_parse(fd, handler, ctx)) {
		L("error parsing '%s'", filename);
		return false;
	}

	return true;
}
