#include "posix.h"

#include <math.h>
#include <stdlib.h>

#include "client/input/cmdline.h"
#include "client/ui/opengl/cmdline.h"
#include "shared/util/log.h"

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

	ctx->sun_theta = (24 - (hours - 12)) * 2.0f * PI / 24.0f;

output:
	hours = (24 - (ctx->sun_theta * 24.0f  / (2.0f * PI))) + 12;

	float mins = (hours - floorf(hours)) * 60.0f;

	snprintf(cmd->out, CMDLINE_BUF_LEN, "% 2.0f:%02.0f", hours, mins);

	return cmdres_ok;
}

struct cmd_table cmds[] = {
	"time", (cmdfunc)cmd_time
};
size_t cmds_len = sizeof(cmds) / sizeof(cmds[0]);

enum cmd_result
opengl_ui_cmdline_hook(struct cmd_ctx *cmd, struct opengl_ui_ctx *ctx,
	struct hiface *hf)
{
	cmdfunc action;

	if ((action = cmd_lookup(cmd, cmds, cmds_len))) {
		return action(cmd, ctx);
	}

	return cmdres_not_found;
}
