#ifndef CLIENT_UI_NCURSES_UI_H
#define CLIENT_UI_NCURSES_UI_H

#include "client/client.h"
#include "client/ui/ncurses/container.h"

struct ncurses_ui_ctx {
	struct display_container dc;
};

bool ncurses_ui_init(struct ncurses_ui_ctx *ctx);
void ncurses_ui_render(struct ncurses_ui_ctx *ctx, struct client *cli);
void ncurses_ui_handle_input(struct ncurses_ui_ctx *ctx, struct client *cli);
struct rectangle ncurses_ui_viewport(struct ncurses_ui_ctx *nc);
void ncurses_ui_deinit(void);
#endif
