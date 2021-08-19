#ifndef LAUNCHER_UI_H
#define LAUNCHER_UI_H

#include "launcher/launcher.h"

struct launcher_ui_ctx {
	struct gl_win *win;
	bool run, exit;

	struct opts *opts;

	enum exit_reason exit_reason;
};

void launcher_ui_init(struct launcher_ui_ctx *ctx, struct opts *opts);
void launcher_ui_render(struct launcher_ui_ctx *ctx);
#endif
