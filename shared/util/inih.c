#include "posix.h"

#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include "shared/math/geom.h"
#include "shared/util/inih.h"
#include "shared/util/log.h"
#include "shared/util/text.h"

struct each_line_ctx {
	char err[INIH_ERR_LEN];
	void *octx;
	char *sect;
	inihcb cb;
	uint32_t line;
	bool success;
};

static enum iteration_result
each_line_cb(void *_ctx, char *line, size_t len)
{
	struct each_line_ctx *ctx = _ctx;
	char *ptr, *key, *val, *err = ctx->err;

	if (*line == '\0' || *line == ';') {
		goto done_with_line;
	} else if (*line == '[') {
		if (!(ptr = strchr(line, ']'))) {
			INIH_ERR("expected ']' at line %d", ctx->line);
			ctx->success = false;
			goto done_with_line;
		}

		*ptr = '\0';

		ctx->sect = line + 1;
		goto done_with_line;
	}

	if (!(ptr = strchr(line, '='))) {
		INIH_ERR("expected '=' at line %d", ctx->line);
		ctx->success = false;
		goto done_with_line;
	}

	*ptr = '\0';

	key = line;
	val = ptr - 1;
	while (is_whitespace(*val)) {
		*val = '\0';
		--val;
	}

	val = ptr + 1;
	while (is_whitespace(*val)) {
		++val;
	}

	if (!ctx->cb(ctx->octx, err, ctx->sect, key, val, ctx->line)) {
		ctx->success = false;
	}

done_with_line:
	if (!ctx->success) {
		return ir_done;
	}

	++ctx->line;

	return ir_cont;
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

	if (!ctx.success) {
		LOG_W(log_misc, "%s:%d error: %s", fd->path, ctx.line, ctx.err);
	}

	return ctx.success;
}

int32_t
cfg_string_lookup_n(const char *str, const struct cfg_lookup_table *tbl, uint32_t n)
{
	size_t i, len;

	for (i = 0; i < CFG_LOOKUP_TBL_LEN; ++i) {
		if (tbl->e[i].str == NULL) {
			break;
		} else {
			len = strlen(tbl->e[i].str);

			if (len == n && (strncmp(tbl->e[i].str, str, n) == 0)) {
				return tbl->e[i].t;
			}
		}
	}

	return -1;
}

int32_t
cfg_string_lookup(const char *str, const struct cfg_lookup_table *tbl)
{
	return cfg_string_lookup_n(str, tbl, strlen(str));
}

bool
parse_cfg_file(const char *filename, void *ctx, inihcb handler)
{
	struct file_data *fd;

	if (!(fd = asset(filename))) {
		return false;
	}

	if (!ini_parse(fd, handler, ctx)) {
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
