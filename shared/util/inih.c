#include "posix.h"

#include <ctype.h>
#include <string.h>

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


