#include "posix.h"

#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include "shared/math/geom.h"
#include "shared/util/inih.h"
#include "shared/util/log.h"
#include "shared/util/text.h"

struct each_line_ctx {
	void *octx;
	char *sect;
	inihcb cb;
	uint32_t line;
	bool success;
};

static void
each_line_cb(void *_ctx, char *line, size_t len)
{
	struct each_line_ctx *ctx = _ctx;
	char *ptr, *key, *val;

	if (*line == '\0' || *line == ';') {
		goto done_with_line;
	} else if (*line == '[') {
		if (!(ptr = strchr(line, ']'))) {
			LOG_W("expected ']' at line %d", ctx->line);
			ctx->success = false;
			goto done_with_line;
		}

		*ptr = '\0';

		ctx->sect = line + 1;
		goto done_with_line;
	}

	if (!(ptr = strchr(line, '='))) {
		LOG_W("expected '=' at line %d", ctx->line);
		ctx->success = false;
		goto done_with_line;
	}

	*ptr = '\0';

	key = line;
	val = ptr - 1;
	while (isspace(*val)) {
		*val = '\0';
		--val;
	}

	val = ptr + 1;
	while (isspace(*val)) {
		++val;
	}

	ctx->cb(ctx->octx, ctx->sect, key, val, ctx->line);

done_with_line:
	++ctx->line;
}

bool
ini_parse(struct file_data *fd, inihcb cb, void *octx)
{
	struct each_line_ctx ctx = {
		.octx = octx,
		.cb = cb,
		.line = 1,
		.success = true,
	};

	each_line(fd, &ctx, each_line_cb);

	return ctx.success;
}

int32_t
cfg_string_lookup(const char *str, struct cfg_lookup_table *tbl)
{
	size_t i;

	for (i = 0; i < CFG_LOOKUP_TBL_LEN; ++i) {
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

float
strdeg_to_rad(const char *str)
{
	return strtof(str, NULL) * PI / 180;
}

bool
str_to_bool(const char *str)
{
	return strcmp(str, "on") == 0 || strcmp(str, "true") == 0;
}
