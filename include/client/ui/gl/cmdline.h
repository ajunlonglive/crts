#ifndef CLIENT_UI_OPENGL_CMDLINE_H
#define CLIENT_UI_OPENGL_CMDLINE_H

#include "client/ui/gl/ui.h"

enum cmd_result gl_ui_cmdline_hook(struct cmd_ctx *cmd, struct gl_ui_ctx *ctx,
	struct client *cli);
#endif
