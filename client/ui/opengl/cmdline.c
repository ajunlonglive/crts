#include "posix.h"

#include <math.h>
#include <stdlib.h>

#include "client/cmdline.h"
#include "client/ui/opengl/cmdline.h"
#include "client/ui/opengl/globals.h"
#include "shared/util/log.h"

static enum cmd_result
cmd_mark(struct cmd_ctx *cmd, struct opengl_ui_ctx *ctx)
{

	int8_t opt;
	struct point pos = { 0 };
	struct { bool index, rel; } opts = { 0 };

	optind = 0;
	while ((opt = getopt(cmd->argc, cmd->argv, "ir")) != -1) {
		switch (opt) {
		case 'r':
			opts.rel = true;
			break;
		case 'i':
			opts.index = true;
			opts.rel = true;
			break;
		default:
			goto argerror;
		}
	}

	if (opts.index) {
		if (cmd->argc - optind != 1) {
			goto argerror;
		}
		uint32_t idx = strtol(cmd->argv[optind], NULL, 10);

		if (idx >= 256) {
			snprintf(cmd->out, CMDLINE_BUF_LEN, "index out of range");
			return cmdres_arg_error;
		}

		pos.x = idx >> 4;
		pos.y = idx & 15;
	} else {
		if (cmd->argc - optind == 0 && !opts.rel) {
			pos = point_add(&ctx->cli->view, &ctx->cli->cursor);
		} else if (cmd->argc - optind == 2) {
			pos.x = strtol(cmd->argv[optind], NULL, 10);
			pos.y = strtol(cmd->argv[optind + 1], NULL, 10);
		} else {
			goto argerror;
		}
	}

	if (opts.rel) {
		struct point cp = point_add(&ctx->cli->view, &ctx->cli->cursor);
		cp = nearest_chunk(&cp);

		pos.x += cp.x;
		pos.y += cp.y;
	}

#ifndef NDEBUG
	darr_push(&ctx->debug_hl_points, &pos);
#endif

	return cmdres_ok;
argerror:
	snprintf(cmd->out, CMDLINE_BUF_LEN, "usage: mark [-r] X Y | mark -i INDEX");
	return cmdres_arg_error;
}

static enum cmd_result
cmd_time(struct cmd_ctx *cmd, struct opengl_ui_ctx *ctx)
{
	float hours = 0.0;

	if (cmd->argc < 2) {
		goto output;
	} else {
		hours = strtol(cmd->argv[1], NULL, 10) % 24;
	}

	if (cmd->argc > 2) {
		hours += (strtol(cmd->argv[2], NULL, 10) % 60) / 60.0f;
	}

	ctx->time.sun_theta_tgt = (24 - (hours - 12)) * 2.0f * PI / 24.0f;

output:
	hours = (24 - (ctx->time.sun_theta_tgt * 24.0f  / (2.0f * PI))) + 12;

	float mins = (hours - floorf(hours)) * 60.0f;

	snprintf(cmd->out, CMDLINE_BUF_LEN, "% 2.0f:%02.0f", hours, mins);

	return cmdres_ok;
}

struct cmd_table cmds[] = {
	"time", (cmdfunc)cmd_time,
	"m", (cmdfunc)cmd_mark,
	"mark", (cmdfunc)cmd_mark,
};
size_t cmds_len = sizeof(cmds) / sizeof(cmds[0]);

enum cmd_result
opengl_ui_cmdline_hook(struct cmd_ctx *cmd, struct opengl_ui_ctx *ctx,
	struct client *cli)
{
	cmdfunc action;

	if ((action = cmd_lookup(cmd, cmds, cmds_len))) {
		return action(cmd, ctx);
	}

	return cmdres_not_found;
}
