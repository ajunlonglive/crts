#ifndef LAUNCHER_UI_H
#define LAUNCHER_UI_H
#include "shared/ui/gl/menu.h"

struct launcher_ui_ctx {
	struct menu_ctx menu;
	struct gl_win *win;
	bool stop, multiplayer, settings;

	struct menu_slider_ctx volume;
};

void launcher_ui_init(struct launcher_ui_ctx *ctx);
void launcher_ui_render(struct launcher_ui_ctx *ctx);
#endif
