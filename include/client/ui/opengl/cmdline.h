#ifndef CLIENT_UI_OPENGL_CMDLINE_H
#define CLIENT_UI_OPENGL_CMDLINE_H

#include "client/input/cmdline.h"
#include "client/ui/opengl/ui.h"

enum cmd_result opengl_ui_cmdline_hook(struct cmd_ctx *cmd, struct opengl_ui_ctx *ctx,
	struct client *cli);
#endif
